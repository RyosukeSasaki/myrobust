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

#define BUF_LEN 1500
#define HISTORY_HASH_SIZE 10
#define HISTORY_LIST_SIZE 30

struct addrinfo *dst_info;
int sfd;
int sequence;

typedef struct _history_list {
    int sequence;
    int length;
    robust_message_t msg;
} history_list_t;
history_list_t history_list[HISTORY_HASH_SIZE][HISTORY_LIST_SIZE];

char* sdata_dir = "./data/";
char* file_name_prefix = "data";

int set_block(int, int);
void skt_config();
void read_dir_file(char *);

int store_file(char *, file_buf_t *);
int send_msg(robust_message_t *);
int send_buf(uint8_t *, int);
int send_file();
int catch_nack();

void init_list();
history_list_t *old_history(int);
history_list_t *search_history(int);

#endif