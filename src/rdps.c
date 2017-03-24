#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>


#include "../util/util.h"

// UDP Client
int send_port;
char* send_ip;
int recv_port;
char* recv_ip;

int main(int argc, char* argv[])
{

  send_ip = argv[1];
  send_port = atoi(argv[2]);
  recv_ip = argv[3];
  recv_port = atoi(argv[4]);
  file_name = argv[5];
  file_pointer = fopen(file_name, "r");

  memset(&stats, 0, sizeof stats);

  bind_socket(send_port, send_ip);

  srand(time(NULL));
  init_seq_num = rand()%100000;

  if (!initialise_queue()) {
    fprintf(stderr, "Error: Couldn't allocate Packet Queue\n");
    free_and_close();
    exit(EXIT_FAILURE);
  }

  memset(&sockaddr_other, 0, sizeof sockaddr_other);
  sockaddr_other.sin_family = AF_INET;
  sockaddr_other.sin_addr.s_addr = inet_addr(recv_ip);
  sockaddr_other.sin_port = htons(recv_port);

  do {
    state = HIP;
    send_syn();
    fd_set file_descriptor_set;
    FD_ZERO(&file_descriptor_set);
    FD_SET(sock, &file_descriptor_set);
    struct timeval tv = {0,24000};
    if (select(sock+1, &file_descriptor_set, NULL, NULL, &tv) == -1) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      if (FD_ISSET(sock,&file_descriptor_set)) {
        packet recv_pkt;
        recsize = recvfrom(sock, (void *)recv_pkt.buf, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr *)&sockaddr_other, &fromlen);
        if (recsize < 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          exit(EXIT_FAILURE);
        } else if (recsize > 0) {
          handle_packet(recv_pkt);
        }
      }
    }
    repeat = 1;
  } while (state==HIP);
  repeat = 0;

  while(!last_packet_acked) {
    fd_set file_descriptor_set;
    FD_ZERO(&file_descriptor_set);
    FD_SET(sock, &file_descriptor_set);
    struct timeval tv = {0,10};
    if (select(sock+1, &file_descriptor_set, NULL, NULL, &tv) == -1) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      if (FD_ISSET(sock,&file_descriptor_set)) {
        packet recv_pkt;
        recsize = recvfrom(sock, (void *)recv_pkt.buf, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr *)&sockaddr_other, &fromlen);
        if (recsize < 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          exit(EXIT_FAILURE);
        } else if (recsize > 0) {
          handle_packet(recv_pkt);
        }
      } else {
        filter_OB();
      }
    }
  }

  char unfinished = 1;
  do {
    send_fin();
    fd_set file_descriptor_set;
    FD_ZERO(&file_descriptor_set);
    FD_SET(sock, &file_descriptor_set);
    struct timeval tv = {0,20000};
    if (select(sock+1, &file_descriptor_set, NULL, NULL, &tv) == -1) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      if (FD_ISSET(sock,&file_descriptor_set)) {
        packet recv_pkt;
        recsize = recvfrom(sock, (void *)recv_pkt.buf, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr *)&sockaddr_other, &fromlen);
        if (recsize < 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          exit(EXIT_FAILURE);
        } else if (recsize > 0) {
          log_event(r,recv_pkt);
          if (recv_pkt._type_ == ACK || recv_pkt._type_ == RST)
            stats.ack++;
            unfinished = 0;
        }
      }
    }
    repeat = 1;
  } while (unfinished);
  print_statistics(1);
  free_and_close();
  return 0;
}
