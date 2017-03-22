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
char *recv_ip;
char *file_name;
FILE *file_pointer;

int main(int argc, char *argv[])
{
  recv_ip = argv[1];
  recv_port = atoi(argv[2]);
  file_name = argv[3];
  file_pointer = fopen(file_name, "w");

  char buffer[MAXIMUM_SEGMENT_SIZE];

  bind_socket(recv_port, recv_ip);
  ssize_t recsize;
  socklen_t fromlen;

  fromlen = sizeof(struct sockaddr_in);

  for (;;)
  {
    recsize = recvfrom(sock, (void *)buffer, MAXIMUM_SEGMENT_SIZE, 0, (struct sockaddr *)&sockaddr_other, &fromlen);
    if (recsize < 0)
    {
      fprintf(stderr, "%s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }
    if (recsize > 0)
    {
      packet mypacket;
      memcpy(mypacket.buf, buffer, MAXIMUM_SEGMENT_SIZE);
      printf("datagram magic: %s\n", mypacket._magic_);
      printf("datagram type: %d\n", (int)mypacket._type_);
      printf("datagram seq: %d\n", mypacket._seqno_or_ackno_);
      printf("datagram len: %d\n", mypacket._length_or_size_);
      printf("datagram magic: %s\n", mypacket._data_);
    }
  }
  return 0;
}
