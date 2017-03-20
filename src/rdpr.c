/*
 *  This file is based off of the upd_server code provided by the lab instructor.
 *  Author: Maxwell Borden
*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* for close() for socket */ 
#include <stdlib.h>

#include "../util/util.h"

// UDP Server
int recv_port;
char* recv_ip;
char* file_name;
FILE* file_pointer;

int main(int argc, char* argv[])
{
  recv_ip = argv[1];
  recv_port = atoi(argv[2]);
  file_name = argv[3];
  file_pointer = fopen(file_name, "w");

  int sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  struct sockaddr_in sa; 
  char buffer[MAXIMUM_SEGMENT_SIZE];
  ssize_t recsize;
  socklen_t fromlen;

  memset(&sa, 0, sizeof sa);
  sa.sin_family = AF_INET;
  sa.sin_addr.s_addr = inet_addr(recv_ip);
  sa.sin_port = htons(recv_port);
  fromlen = sizeof(sa);

  if (-1 == bind(sock, (struct sockaddr *)&sa, sizeof sa)) {
    perror("error bind failed");
    close(sock);
    exit(EXIT_FAILURE);
  }

  for (;;) {
    recsize = recvfrom(sock, (void*)buffer, sizeof buffer, 0, (struct sockaddr*)&sa, &fromlen);
    if (recsize < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    printf("datagram: %.*s\n", (int)recsize, buffer);
  }
}

