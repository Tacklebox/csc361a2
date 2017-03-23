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

  srand(time(NULL));
  init_seq_num = rand();

  bind_socket(send_port, send_ip);

  memset(&sockaddr_other, 0, sizeof sockaddr_other);
  sockaddr_other.sin_family = AF_INET;
  sockaddr_other.sin_addr.s_addr = inet_addr(recv_ip);
  sockaddr_other.sin_port = htons(recv_port);

  packet mypacket;
  mypacket._type_ = DAT;
  mypacket._seqno_or_ackno_ = 1337;
  mypacket._length_or_size_ = 11;
  strcpy(mypacket._data_, "0123456789");
  send_packet(mypacket);

  close(sock); /* close the socket */
  return 0;
}
