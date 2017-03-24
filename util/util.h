#ifndef UTIL_H
#define UTIL_H
#define MAGIC_LENGTH 6
#define MAXIMUM_SEGMENT_SIZE 1024
#define RDP_HEADER_SIZE 20
#define DATA_LENGTH 1004
#include "pqueue.h"

typedef enum connection_state {
  IDLE,
  HIP,
  CONN,
  FIP,
  TWAIT
} connection_state;

typedef struct con_stat {
  unsigned int total_data;
  unsigned int unique_data;
  unsigned int total_packets;
  unsigned int unique_packets;
  unsigned int syn;
  unsigned int fin;
  unsigned int ack;
  unsigned long long start_time;
} con_stat;

typedef enum packet_type {
  DAT,
  ACK,
  SYN,
  FIN,
  RST
} packet_type;

typedef enum event_type {
  s,
  S,
  r,
  R
} event_type;

typedef union packet {
  struct {
    char _magic_[MAGIC_LENGTH+1]; //"CSC361"
    packet_type _type_;           //From ENUM above
    unsigned int _seqno_or_ackno_;         //Sequence Number if SYN, FYN, or DAT Acknowledge Number if ACK
    int _length_or_size_;         //Payload Length if DAT Window Size if ACK
    char _data_[DATA_LENGTH];     //Actual Data
  };

  char buf[MAXIMUM_SEGMENT_SIZE];
} packet;

typedef struct node_t
{
  pqueue_pri_t pri;
  packet pkt;
  size_t pos;
} node_t;

int bind_socket(int, char*);
int initialise_queue();
void filter_IB();
void filter_OB();
void reset_connection();
void free_and_close();
void handle_packet(packet);
void log_event(event_type, packet);
void make_packet(packet*, packet_type, int, char*, int);
void send_syn();
void send_fin();
void print_statistics(char);
void send_ack(int);
void send_packet(packet);

extern int sock, window_size;
extern unsigned int init_seq_num;
extern struct sockaddr_in sockaddr_self, sockaddr_other;
extern connection_state state;
extern char* file_name;
extern FILE* file_pointer;
extern pqueue_t *pq;
extern node_t   *n;
extern ssize_t recsize;
extern socklen_t fromlen;
extern con_stat stats;
extern char last_packet_acked, repeat, new_packets;

#endif
