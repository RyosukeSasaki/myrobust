#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

#define DST_PORT "50012"
#define DST_ADDR "localhost"
#define BUF_LEN 1500
#define PAYLOAD_MAX 1472
#define HEADER_SIZE 6
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

typedef struct _filebuf {
    int pos;
    int size;
    uint16_t fileno;
    uint8_t buf[FILE_SIZE];
} file_buf_t;

#endif