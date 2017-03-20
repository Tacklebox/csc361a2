#define MAGIC_LENGTH 6
#define MAXIMUM_SEGMENT_SIZE 1024

typedef enum packet_type {
  DAT,
  ACK,
  SYN,
  FIN,
  RST
} packet_type;

typedef struct packet_header {
  char _magic_[MAGIC_LENGTH+1]; //CSC361
  enum packet_type _type_;      // From ENUM above
  int _seqno_or_ackno_;         //Sequence Number if SYN, FYN, or DAT Acknowledge Number if ACK
  int _length_or_size_;         //Payload Length if DAT Window Size if ACK
} packet_header;

typedef union packet_header_u {
  struct packet_header ph;
  char* raw[sizeof(struct packet_header)];
} packet_header_u;

packet_header create_packet_header(int seq, packet_type type)