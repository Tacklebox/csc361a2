#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../util/util.h"

// UDP Client
int send_port;
char* send_ip;
int recv_port;
char* recv_ip;
char* file_name;
FILE* file_pointer;

int main(int argc, char* argv[])
{
  
  send_ip = argv[1];
  send_port = atoi(argv[2]);
  recv_ip = argv[3];
  recv_port = atoi(argv[4]);
  file_name = argv[5];
  file_pointer = fopen(file_name, "w");

  int sock;
  struct sockaddr_in sa;
  int bytes_sent;
  char buffer[MAXIMUM_SEGMENT_SIZE];
 
  strcpy(buffer, "hello world!");
 
  /* create an Internet, datagram, socket using UDP */
  sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (-1 == sock) {
      /* if socket failed to initialize, exit */
      printf("Error Creating Socket");
      exit(EXIT_FAILURE);
    }
 
  /* Zero out socket address */
  memset(&sa, 0, sizeof sa);
  
  /* The address is IPv4 */
  sa.sin_family = AF_INET;
 
   /* IPv4 adresses is a uint32_t, convert a string representation of the octets to the appropriate value */
  sa.sin_addr.s_addr = inet_addr(recv_ip);
  
  /* sockets are unsigned shorts, htons(x) ensures x is in network byte order, set the port to 7654 */
  sa.sin_port = htons(recv_port);
  packet mypacket;
  strcpy(mypacket._magic_, "CSC361"); 
  mypacket._type_ = DAT;
  mypacket._seqno_or_ackno_ = 1337;
  mypacket._length_or_size_ = 11;
  strcpy(mypacket._data_, "0123456789"); 
  bytes_sent = sendto(sock, mypacket.buf, MAXIMUM_SEGMENT_SIZE, 0,(struct sockaddr*)&sa, sizeof sa);
  if (bytes_sent < 0) {
    printf("Error sending packet: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }
 
  close(sock); /* close the socket */
  return 0;
}