#ifndef PACKET_H_
#define PACKET_H_
#define MAXIMUM_SEGMENT_SIZE 1024
#define MAXIMUM_PAYLOAD_LENGTH 984

#include <sys/time.h>

typedef struct timeval ts;

typedef enum packet_type {
    DAT,
    ACK,
    S_ACK,
    SYN,
    FIN,
    RST
} packet_type;

typedef union packet {
    struct {
        char magic[MAGIC_LENGTH+1]; //"CSC361"
        packet_type type;           //From ENUM above
        union {
            unsigned int sequence_number;         //Sequence Number if SYN, FYN, or DAT Acknowledge Number if ACK
            unsigned int acknowledge_number;         //Sequence Number if SYN, FYN, or DAT Acknowledge Number if ACK
        };
        union {
            unsigned short payload_length;         //Payload Length if DAT Window Size if ACK
            unsigned short window_size;         //Payload Length if DAT Window Size if ACK
        };
        ts time_stamp;
        char payload[MAXIMUM_PAYLOAD_LENGTH];     //Actual Data
    };

    char serialized_buffer[MAXIMUM_SEGMENT_SIZE];
} packet;

#endif
