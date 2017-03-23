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
unsigned int init_seq_num, seq_edge, g_ack;
struct sockaddr_in sockaddr_self, sockaddr_other;
connection_state state;
char* file_name;
FILE* file_pointer;
char* packet_type_strings[] = {"DAT", "ACK", "SYN", "FIN", "RST"};
char* event_strings[] = {"s","S","r","R"};
int fast_retransmit_counter = 0;

int bind_socket(int port, char* ip) {
  int option = 1;

  if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    close(sock);
    exit(EXIT_FAILURE);
  }

  memset(&sockaddr_self, 0, sizeof sockaddr_self);
  sockaddr_self.sin_family = AF_INET;
  sockaddr_self.sin_addr.s_addr = inet_addr(ip);
  sockaddr_self.sin_port = htons(port);

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option) == -1) {
    fprintf(stderr, "Error: setsockopt() unsuccessful\n");
    close(sock);
    exit(EXIT_FAILURE);
  }

  if (bind(sock, (struct sockaddr *)&sockaddr_self, sizeof sockaddr_self) == -1) {
    fprintf(stderr, "Error: bind() unsuccessful\n");
    close(sock);
    exit(EXIT_FAILURE);
  }

  return 0;
}

int make_next_dat_packet(packet* p) {
  char p_d[DATA_LENGTH];
  int p_d_l;
  if (file_pointer != NULL) {
    if ((p_d_l = fread(p_d, 1, DATA_LENGTH, file_pointer)) <= 0) {
      //TODO: spam RST
      close(sock);
      fprintf(stderr, "Error: File I/O error\n");
      exit(EXIT_FAILURE);
    } else if (p_d_l == 0) {
      return 0;
    }
    make_packet(p, DAT, seq_edge+1, p_d, p_d_l);
    seq_edge += p_d_l;
  }
  return 1;
}

void make_packet(packet* p, packet_type p_t, int s_a, char* p_d, int p_d_l) {
  strcpy(p->_magic_, "CSC361");
  p->_type_ = p_t;
  p->_seqno_or_ackno_ = s_a;
  if (p_t == ACK) {
    p->_length_or_size_ = window_size;
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
    close(sock);
    printf("Error sending packet: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void send_ack(int ack_no) {
  packet pkt;
  make_packet(&pkt,ACK,1,NULL,0);
  send_packet(pkt);
}

void write_packet_to_file(packet pkt) {
  if (file_pointer != NULL) {
    if (fread(pkt._data_, pkt._length_or_size_, 1, file_pointer) != 1) {
      return;
    }
  }
  //TODO: spam RST
  close(sock);
  fprintf(stderr, "Error: File I/O error\n");
  exit(EXIT_FAILURE);
}

void handle_packet(packet pkt) {
  if (strncmp(pkt._magic_,"CSC361",MAGIC_LENGTH)) {
    fprintf(stderr, "Error: Not a magic packet\n");
    close(sock);
    exit(EXIT_FAILURE);
  }
  switch(pkt._type_) {
    case DAT:
      if (state == HIP) {
        state=CONN;
        //TODO: start a timer for stats
      } else if (state != CONN) {
        fprintf(stderr, "Error: Out of bounds packet\n");
        close(sock);
        exit(EXIT_FAILURE);
      }
      log_event(r,pkt);
      write_packet_to_file(pkt);
      send_ack(pkt._seqno_or_ackno_ + pkt._length_or_size_); //TODO: add cumulative acks and error control for dropped or ooo packets 
      break;

    case ACK:
      if (pkt._seqno_or_ackno_ == g_ack) {
        fast_retransmit_counter++;
      } else if (pkt._seqno_or_ackno_ < g_ack) {
        fast_retransmit_counter = 0;
      } else {
        fast_retransmit_counter = 0;
        g_ack = pkt._seqno_or_ackno_;
        window_size = pkt._length_or_size_;
      }
      break;

    case SYN:
      init_seq_num = pkt._seqno_or_ackno_;
      send_ack(1);
      state = HIP;
      break;

    case FIN:
      state = TWAIT;
      send_ack(pkt._seqno_or_ackno_+1);
      break;

    case RST:
      fprintf(stderr, "Error: Connection Reset\n");
      close(sock);
      exit(EXIT_FAILURE);
      break;

    default:
      fprintf(stderr, "Error: Unknown or malformed packet _type_\n");
      close(sock);
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

/*
 * pqueue.c and pqueue.h are from https://github.com/vy/libpqueue
 * the license to use them are in their respective files
 * Functions for apache PRIO Q
 */

static int cmp_pri(pqueue_pri_t next, pqueue_pri_t curr) {
  return (next < curr);
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

