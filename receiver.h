#ifndef RECEIVER_H
#define RECEIVER_H

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
#include <sys/socket.h>
#include <signal.h>
#include <sys/time.h>

#define BUF_LEN 1500
#define FILE_LIST_SIZE 5
#define STAT_EMPTY -1
#define STAT_CURRENT 0
#define STAT_OLD 1
#define STAT_DONE 2

int sfd, running=1, alm_flag=0;
struct sockaddr_in fromaddr;
socklen_t addrsize = sizeof(fromaddr);

typedef struct _file_list {
    file_buf_t buf;
    int fileno;
    int8_t stat;
    int8_t arrived[70];
} file_list_t;

char* sdata_dir = "./data/";
char* file_name_prefix = "data";

void signal_config();
void sigint_handler();
void sigalrm_handler();
void alarm_config();

int get_msg(struct sockaddr_in *, socklen_t *, robust_message_t *);
int get_file();
int save_file(file_buf_t *, int);
int send_nak();

void init_list();
void skt_config();
int all_arrived();
int send_nack(struct sockaddr_in *, socklen_t *);
file_list_t *old_file();
file_list_t *search_file(int);

#endif