#define MAGIC_LENGTH 6
#define MAXIMUM_SEGMENT_SIZE 1024
#define DATA_LENGTH 1004

int bind_socket(int*, int, char*);

typedef enum packet_type {
  DAT,
  ACK,
  SYN,
  FIN,
  RST
} packet_type;

typedef union packet {
  struct {
  char _magic_[MAGIC_LENGTH+1]; //"CSC361"
  packet_type _type_;           //From ENUM above
  int _seqno_or_ackno_;         //Sequence Number if SYN, FYN, or DAT Acknowledge Number if ACK
  int _length_or_size_;         //Payload Length if DAT Window Size if ACK
  char _data_[DATA_LENGTH];     //Actual Data
  };
  char buf[MAXIMUM_SEGMENT_SIZE];
} packet;

/*
typdef struct packet_queue {
  packet* this;
  struct packet_queue* next;
} packet_queue;
*/

//packet_header create_packet(int seq, packet_type type)
