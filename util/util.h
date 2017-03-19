enum packet_type {
  DAT,
  ACK,
  SYN,
  FIN,
  RST
};

#define MAGIC_LENGTH 6
typedef struct packet_header {
  char _magic_[MAGIC_LENGTH+1]; //CSC361
  enum packet_type _type_; // From ENUM above
  int _seqno_;             //Sequence Number if SYN, FYN, or DAT
  int _ackno_;             //Acknowledge Number if ACK
  int _length_;            //Payload Length if DAT
  int _size_;              //Window Size if ACK
} packet_header;

struct packet_header packet_header;

typedef union packet_header_u {
  struct packet_header ph;
  char* raw[sizeof(struct packet_header)];
} packet_header_u;