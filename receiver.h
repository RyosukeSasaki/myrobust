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

#define BUF_LEN 1500

int sfd, running=1;
struct sockaddr_in fromaddr;
socklen_t addrsize = sizeof(fromaddr);

char* sdata_dir = "./data/";
char* file_name_prefix = "data";

void skt_config();
void signal_config();
void sigint_handler();
int get_msg(struct sockaddr_in *fromaddr, socklen_t *addrsize, robust_message_t *msg);
int get_file();
int save_file(file_buf_t *, int);
int send_nak();

#endif