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

int sock;
struct sockaddr_in sockaddr_self, sockaddr_other;
connection_state state;

//packet make_packet(packet_type type, int sack_number, int paydow_length){
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

void log_event(event_type e, packet pkt){

}
