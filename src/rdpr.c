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
  struct sockaddr_in server_addr, client_addr;
  char buffer[MAXIMUM_SEGMENT_SIZE];
  ssize_t recsize;
  socklen_t fromlen;

  memset(&server_addr, 0, sizeof server_addr);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(recv_ip);
  server_addr.sin_port = htons(recv_port);
  fromlen = sizeof(server_addr);

  if (-1 == bind(sock, (struct sockaddr *)&server_addr, sizeof server_addr)) {
    perror("error bind failed");
    close(sock);
    exit(EXIT_FAILURE);
  }

  for (;;) {
    recsize = recvfrom(sock, (void*)buffer, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr*)&server_addr, &fromlen);
    if (recsize < 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    packet mypacket;
    memcpy(mypacket.buf, buffer, MAXIMUM_SEGMENT_SIZE);
    if (mypacket._type_ == DAT) {
      printf("datagram type is DAT!\n");
    }
    printf("datagram magic: %s\n", mypacket._magic_);
    printf("datagram type: %d\n", (int)mypacket._type_);
    printf("datagram seq: %d\n", mypacket._seqno_or_ackno_);
    printf("datagram len: %d\n", mypacket._length_or_size_);
    printf("datagram magic: %s\n", mypacket._data_);
  }
  return 0;
}

