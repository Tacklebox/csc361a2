enum packet_type {
  DAT,
  ACK,
  SYN,
  FIN,
  RST
};

typedef struct packet_header {
  char* _magic_; //CSC361
  enum packet_type _type_; // From ENUM above
  union {
    int _seqno_; //Sequence Number if SYN, FYN, or DAT
    int _ackno_; //Acknowledge Number if ACK
  };
  union {
    int _length_; //Payload Length if DAT
    int _size_; //Window Size if ACK
  };
} packet_header;
