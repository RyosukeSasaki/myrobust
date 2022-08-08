#ifndef SENDER_H
#define SENDER_H

#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>

struct addrinfo *dst_info;
int sfd;

typedef struct _file_buf_list {
    struct _file_buf_list *fp;
    struct _file_buf_list *bp;
    int sent;
    file_buf_t buf;
} file_buf_list_t;
file_buf_list_t file_buf_head;
file_buf_list_t file_buf_list[10];

char* sdata_dir = "./data/";
char* file_name_prefix = "data";

void skt_config();
void read_dir_file(char *);
int store_file(char *, file_buf_t *);
int send_msg(robust_message_t *);
int send_buf(uint8_t *, int);
int send_file();

void init_list();
void insert_head(file_buf_list_t *);
void remove_from_list(file_buf_list_t *);
int search_file();

#endif