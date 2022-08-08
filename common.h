#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define DST_PORT "50012"
#define DST_ADDR "localhost"
#define PAYLOAD_MAX 1472
#define HEADER_SIZE 4
#define FILE_SIZE 102400
#define FILENUM 999
#define PATH_MAX 4096
#define DATA_MAX (PAYLOAD_MAX-HEADER_SIZE)

typedef union _robust_message {
    struct message {
        uint16_t sequence;
        uint16_t length;
        uint8_t data[DATA_MAX];
    } msg;
    uint8_t data[PAYLOAD_MAX];
} robust_message_t;

#define NACK_HEADER_SIZE 0
#define NACK_MAX (PAYLOAD_MAX-NACK_HEADER_SIZE)

typedef union _robust_nack {
    struct nack {
        uint16_t data[NACK_MAX/2];
    } msg;
    uint8_t data[PAYLOAD_MAX];
} robust_nack_t;

typedef struct _filebuf {
    int pos;
    int size;
    uint8_t buf[FILE_SIZE];
} file_buf_t;

#endif