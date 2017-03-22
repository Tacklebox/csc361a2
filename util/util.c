#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* for close() for socket */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

int sock, window_size, init_seq_num;
struct sockaddr_in sockaddr_self, sockaddr_other;
connection_state state;

//packet make_packet(packet_type type, int sack_number, int paydow_length) {
//}

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

void make_packet(packet* p, packet_type p_t, int s_a, char* p_d, int p_d_l) {
  strcpy(p->_magic_, "CSC361");
  p->_type_ = p_t;
  p->_seqno_or_ackno_ = s_a;
  if(p_t == ACK) {
    p->_length_or_size_ = window_size;
  } else if(p_t == DAT) {
    p->_length_or_size_ = p_d_l;
  }
  return;
}

void send_ack(int ack_no) {
  packet pkt;
  make_packet(&pkt,ACK,1,NULL,0);
  //send_packet(pkt)
}

void handle_packet(packet pkt) {
  if(strncmp(pkt._magic_,"CSC361",MAGIC_LENGTH)) {
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
      break;
    case ACK:
      break;
    case SYN:
      init_seq_num = pkt._seqno_or_ackno_;
      send_ack(1);
      state = HIP;
      break;
    case FIN:
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

void log_event(event_type e, packet pkt) {

}
