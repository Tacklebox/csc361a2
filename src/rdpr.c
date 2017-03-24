/*
 *  This file is based off of the upd_server code provided by the lab instructor.
 *  Author: Maxwell Borden
*/

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> /* for close() for socket */

#include "../util/util.h"

// UDP Server
int recv_port;
char *recv_ip;

int main(int argc, char *argv[]) {
  recv_ip = argv[1];
  recv_port = atoi(argv[2]);
  file_name = argv[3];
  file_pointer = fopen(file_name, "w");
  fromlen = sizeof(struct sockaddr_in);

  memset(&stats, 0, sizeof stats);

  bind_socket(recv_port, recv_ip);

  if (!initialise_queue()) {
    fprintf(stderr, "Error: Couldn't allocate Packet Queue\n");
    free_and_close();
    exit(EXIT_FAILURE);
  }

  window_size = 32;

  state = IDLE;

  while(1) {
    fd_set file_descriptor_set;
    FD_ZERO(&file_descriptor_set);
    FD_SET(sock, &file_descriptor_set);
    struct timeval tv = {0,0};
    tv.tv_usec = (state == TWAIT) ? 50000 : 5000;
    if (select(sock+1, &file_descriptor_set, NULL, NULL, &tv) == -1) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    } else {
      if (FD_ISSET(sock,&file_descriptor_set)) {
        new_packets = 1;
        packet recv_pkt;
        recsize = recvfrom(sock, (void *)recv_pkt.buf, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr *)&sockaddr_other, &fromlen);
        if (recsize < 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          exit(EXIT_FAILURE);
        } else if (recsize > 0) {
          handle_packet(recv_pkt);
        }
      } else if (state != IDLE) {
        if (state == TWAIT) {
          free_and_close();
          exit(EXIT_SUCCESS);
        }
        if (new_packets)
          filter_IB();
        new_packets = 0;
      }
    }
  }
  return 0;
}
