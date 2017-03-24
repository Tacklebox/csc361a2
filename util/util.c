#include <arpa/inet.h>
#include <errno.h>
#include <limits.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "util.h"
#include "pqueue.h"

int sock, window_size;
unsigned int init_seq_num, rseq_edge, sseq_edge, g_ack;
struct sockaddr_in sockaddr_self, sockaddr_other;
connection_state state;
char* file_name;
FILE* file_pointer;
char* packet_type_strings[] = {"DAT", "ACK", "SYN", "FIN", "RST"};
char* event_strings[] = {"s","S","r","R"};
int fast_retransmit_counter = 0;
pqueue_t *pq;
node_t   *n;
ssize_t recsize;
socklen_t fromlen;
char last_data_packet_created = 0, last_packet_acked, repeat;
unsigned long long retransmission_timeout = 1000000;

int bind_socket(int port, char* ip) {
  int option = 1;

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    free_and_close();
    exit(EXIT_FAILURE);
  }

  memset(&sockaddr_self, 0, sizeof sockaddr_self);
  sockaddr_self.sin_family = AF_INET;
  sockaddr_self.sin_addr.s_addr = inet_addr(ip);
  sockaddr_self.sin_port = htons(port);

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option) == -1) {
    fprintf(stderr, "Error: setsockopt() unsuccessful\n");
    free_and_close();
    exit(EXIT_FAILURE);
  }

  if (bind(sock, (struct sockaddr *)&sockaddr_self, sizeof sockaddr_self) == -1) {
    fprintf(stderr, "Error: bind() unsuccessful\n");
    free_and_close();
    exit(EXIT_FAILURE);
  }

  return 0;
}

void free_and_close() {
  pqueue_free(pq);
  free_and_close();
  fclose(file_pointer);
}

int make_next_dat_packet(packet* p) {
  char p_d[DATA_LENGTH];
  int p_d_l;
  if ((p_d_l = fread(p_d, 1, DATA_LENGTH, file_pointer)) < 0) {
    //TODO: spam RST
    free_and_close();
    fprintf(stderr, "Error: File I/O error\n");
    exit(EXIT_FAILURE);
  }
  make_packet(p, DAT, sseq_edge, p_d, p_d_l);
  sseq_edge += p_d_l;
  return (p_d_l < DATA_LENGTH);
}

void make_packet(packet* p, packet_type p_t, int s_a, char* p_d, int p_d_l) {
  strcpy(p->_magic_, "CSC361");
  p->_type_ = p_t;
  p->_seqno_or_ackno_ = s_a;
  if (p_t == ACK) {
    unsigned int roll_remaining = (UINT_MAX - rseq_edge) / MAXIMUM_SEGMENT_SIZE;
    roll_remaining = (roll_remaining)?roll_remaining:1;
    p->_length_or_size_ = (window_size <= roll_remaining)? window_size:roll_remaining;
  } else if (p_t == SYN) {
    p->_length_or_size_ = 0;
  } else if (p_t == DAT && p_d_l > 0) {
    p->_length_or_size_ = p_d_l;
    memcpy(p->_data_, p_d, p_d_l);
  }
  return;
}

void send_packet(packet pkt) {
  int len = (pkt._type_ == DAT)? pkt._length_or_size_+RDP_HEADER_SIZE : RDP_HEADER_SIZE;
  int bytes_sent;
  bytes_sent = sendto(sock, pkt.buf, len, 0,(struct sockaddr*)&sockaddr_other, sizeof (struct sockaddr_in));
  if (bytes_sent < 0) {
    free_and_close();
    printf("Error sending packet: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void send_ack(int ack_no) {
  packet pkt;
  make_packet(&pkt,ACK,ack_no,NULL,0);
  send_packet(pkt);
  if (g_ack < ack_no) {
    g_ack = ack_no;
    log_event(s,pkt);
  } else {
    log_event(S,pkt);
  }
}

void send_syn() {
  packet pkt;
  make_packet(&pkt,SYN,init_seq_num,NULL,0);
  send_packet(pkt);
  if (repeat) {
    log_event(S,pkt);
  } else {
    log_event(s,pkt);
  }
}

void send_fin() {
  packet pkt;
  make_packet(&pkt,FIN,g_ack+1,NULL,0);
  send_packet(pkt);
  if (repeat) {
    log_event(S,pkt);
  } else {
    log_event(s,pkt);
  }
}

void write_packet_to_file(packet pkt) {
  if (file_pointer != NULL) {
    if (fwrite(pkt._data_, pkt._length_or_size_, 1, file_pointer) == 1) {
      return;
    }
  }
  free_and_close();
  fprintf(stderr, "Error: File I/O error\n");
  exit(EXIT_FAILURE);
}

unsigned long long now(){
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return tv.tv_sec*1000000ull+tv.tv_usec;
}

int check_expired(unsigned long long timestamp) {
  return (timestamp+retransmission_timeout < now());
}

void filter_IB() {
  while((n = pqueue_pop(pq))) {
    if (n->pkt._seqno_or_ackno_ == rseq_edge) {
      write_packet_to_file(n->pkt);
      rseq_edge += n->pkt._length_or_size_;
    }
  }
  send_ack(rseq_edge);
}

void filter_OB() {
  if (!last_data_packet_created) {
    while (pqueue_size(pq) < window_size) {
      n = malloc(sizeof(node_t));
      packet tmp;
      last_data_packet_created = make_next_dat_packet(&tmp);
      n->pkt = tmp;
      n->pri = now();
      send_packet(n->pkt);
      log_event(s,n->pkt);
      pqueue_insert(pq, n);
    }
  }
  n = pqueue_peek(pq);
  if(n != NULL && (n->pkt._seqno_or_ackno_ < g_ack || check_expired(n->pri))) {
  }
  while(n != NULL && (n->pkt._seqno_or_ackno_ < g_ack || check_expired(n->pri))) {
    if (n->pkt._seqno_or_ackno_ < g_ack) {
      pqueue_pop(pq);
    } else {
      send_packet(n->pkt);
      log_event(S,n->pkt);
      pqueue_change_priority(pq, now(), n);
    }
    n = pqueue_peek(pq);
  }
}


void reset_connection() {
  packet pkt;
  make_packet(&pkt,RST,0,NULL,0);
  int i;
  for(i = 0; i<3; i++) {
    send_packet(pkt);
  }
}

void handle_packet(packet pkt) {
  if (strncmp(pkt._magic_,"CSC361",MAGIC_LENGTH)) {
    fprintf(stderr, "Error: Not a magic packet\n");
    free_and_close();
    exit(EXIT_FAILURE);
  }
  log_event(r,pkt);
  switch(pkt._type_) {
    case DAT:
      if (state == HIP) {
        state=CONN;
        //TODO: start a timer for stats
      } else if (state != CONN) {
        fprintf(stderr, "Error: Out of bounds packet\n");
        free_and_close();
        exit(EXIT_FAILURE);
      }
      n = malloc(sizeof(node_t));
      n->pri = pkt._seqno_or_ackno_;
      n->pkt = pkt;
      pqueue_insert(pq, n);
      break;

    case FIN:
      if (pkt._seqno_or_ackno_ == rseq_edge) {
        state = TWAIT;
        send_ack(pkt._seqno_or_ackno_+1);
      }
      break;

    case ACK:
      if (pkt._seqno_or_ackno_ == (init_seq_num + 1) && state == HIP) {
        state = CONN;
        sseq_edge = init_seq_num + 1;
      }
      if (pkt._seqno_or_ackno_ == g_ack) {
        fast_retransmit_counter++;
        if (fast_retransmit_counter > 2) {
          //TODO: perform FRT
        }
      } else if (pkt._seqno_or_ackno_ < g_ack) {
        //ignore lower out of order ack
        fast_retransmit_counter = 0;
      } else {
        fast_retransmit_counter = 0;
        g_ack = pkt._seqno_or_ackno_;
        window_size = pkt._length_or_size_;
      }
      break;

    case SYN:
      init_seq_num = pkt._seqno_or_ackno_;
      rseq_edge = init_seq_num + 1;
      send_ack(rseq_edge);
      state = HIP;
      break;

    case RST:
      fprintf(stderr, "Error: Connection Reset\n");
      free_and_close();
      exit(EXIT_FAILURE);
      break;

    default:
      fprintf(stderr, "Error: Unknown or malformed packet _type_\n");
      free_and_close();
      exit(EXIT_FAILURE);
  }
}

//got time printing strategy from http://stackoverflow.com/questions/1551597/using-strftime-in-c-how-can-i-format-time-exactly-like-a-unix-timestamp
void log_event(event_type e, packet pkt) {
  char            fmt[64], buf[64];
  struct timeval  tv;
  struct tm       *tm;

  gettimeofday(&tv, NULL);
  if ((tm = localtime(&tv.tv_sec)) != NULL) {
    strftime(fmt, sizeof fmt, "%H:%M:%S.%%06u", tm);
    snprintf(buf, sizeof buf, fmt, tv.tv_usec);
    printf("%s ", buf);
    printf("%s ", event_strings[e]);
    if (e == s || e == S) {
      printf("%s:%d ", inet_ntoa(sockaddr_self.sin_addr), ntohs(sockaddr_self.sin_port));
      printf("%s:%d ", inet_ntoa(sockaddr_other.sin_addr), ntohs(sockaddr_other.sin_port));
    } else {
      printf("%s:%d ", inet_ntoa(sockaddr_other.sin_addr), ntohs(sockaddr_other.sin_port));
      printf("%s:%d ", inet_ntoa(sockaddr_self.sin_addr), ntohs(sockaddr_self.sin_port));
    }
    printf("%s %d %d\n", packet_type_strings[pkt._type_], pkt._seqno_or_ackno_, pkt._length_or_size_);
  }
}

int get_file_size(char *filename) {
  struct stat st;
  if (stat(filename, &st) < 0) {
    return -1;
  }
  return st.st_size;
}

/*
 * pqueue.c and pqueue.h are from https://github.com/vy/libpqueue
 * the license to use them are in their respective files
 * Functions for apache PRIO Q
 */
static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr) {
  return (next > curr);
}


static pqueue_pri_t get_pri(void *a) {
  return ((node_t *) a)->pri;
}


static void set_pri(void *a, pqueue_pri_t pri) {
  ((node_t *) a)->pri = pri;
}


static size_t get_pos(void *a) {
  return ((node_t *) a)->pos;
}


static void set_pos(void *a, size_t pos) {
  ((node_t *) a)->pos = pos;
}

int initialise_queue() {
  pq = pqueue_init(32, cmp_pri, get_pri, set_pri, get_pos, set_pos);
  return (pq != NULL);
}

