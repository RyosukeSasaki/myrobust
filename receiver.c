#include "receiver.h"

file_list_t file_list[FILE_LIST_SIZE];
file_list_t *current = NULL;

int main()
{
    skt_config();
    signal_config();
    alarm_config();
    while(running) {
        get_file();
    }
    fprintf(stderr, "Exiting...\n");
    close(sfd);
}

int get_file()
{
    robust_message_t msg;
    static file_buf_t *file=NULL;
    int cnt;
    int fileno;
    /**
    current = old_file();
    current->fileno = 0;
    file = &current->buf;
    **/
    while (running) {
        if ((cnt=get_msg(&fromaddr, &addrsize, &msg)) < 0) {
            if (alm_flag) send_nack(&fromaddr, &addrsize);
            alm_flag = 0;
            continue;
        }
        fileno = msg.msg.sequence / 70;

        /**
        **/
        if (current == NULL) {
            // create new (1st time)
            current = old_file();
            current->stat = STAT_CURRENT;
            current->fileno = fileno;
            file = &current->buf;
        } else if (current->fileno != fileno) {
            if (search_file(fileno) == NULL) {
                // create new
                current->stat = STAT_OLD;
                current = old_file();
                current->stat = STAT_CURRENT;
                current->fileno = fileno;
            } else {
                // existing file buf
                current->stat = STAT_OLD;
                current = search_file(fileno);
                current->stat = STAT_CURRENT;
            }
            file = &current->buf;
        }

        file->size += msg.msg.length;
        memcpy(&file->buf[(msg.msg.sequence % 70)*DATA_MAX], msg.msg.data, msg.msg.length);
        current->arrived[msg.msg.sequence % 70] = 1;
        //if ((msg.msg.sequence+1) % 70 == 0) break;
        if (all_arrived()) {
            current->stat = STAT_DONE;
            return save_file(file, fileno);
        }
    }
    //if (file == NULL) return -1;
}

int save_file(file_buf_t *file, int fileno)
{
    char file_path[PATH_MAX];
    //fprintf(stderr, "writing file %d %d\n",fileno, (*file).size);
    memset(&file_path, 0, sizeof(file_path));
    snprintf(file_path, sizeof(file_path), "recv/%s%d", file_name_prefix, fileno);
    FILE *fd;
    if((fd = fopen(file_path, "w")) < 0) {
        perror("fopen");
        fclose(fd);
        return -1;
    }
    fwrite((*file).buf, 1, (*file).size, fd);
    fclose(fd);
    return 0;
}

void skt_config()
{
	struct sockaddr_in addr;
    if ((sfd=socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(DST_PORT));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(-1);
    }
}

void sigint_handler() { running = 0; }

void sigalrm_handler() { alm_flag = 1; }

void signal_config()
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
    sa.sa_handler = sigalrm_handler;
    if (sigaction(SIGALRM, &sa, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
}

int get_msg(struct sockaddr_in *fromaddr, socklen_t *addrsize, robust_message_t *msg) 
{
    uint8_t buf[BUF_LEN];
    struct in_addr addrbuf;
    int cnt;

    memset(buf, 0, sizeof(buf));
    memset(msg->data, 0, sizeof(msg->data));
    if ((cnt = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr *)fromaddr, addrsize)) < 0) {
        //perror("recvfrom");
        return cnt;
    }
    if (cnt == 0) return -1;
    memcpy(msg->data, buf, cnt);
    msg->msg.length = ntohs(msg->msg.length);
    msg->msg.sequence = ntohs(msg->msg.sequence);

    return cnt;
}

file_list_t *old_file()
{
    //fprintf(stderr, "next file\n");
    static int index = 0;
    file_list_t *ret = &file_list[index];
    memset(file_list[index].arrived, 0, sizeof(file_list[index].arrived));
    memset(file_list[index].buf.buf, 0, sizeof(file_list[index].buf.buf));
    file_list[index].buf.size = 0;
    file_list[index].buf.pos = 0;
    file_list[index].stat = STAT_EMPTY;
    file_list[index].fileno = -1;
    index = (index+1) % FILE_LIST_SIZE;
    return ret;
}

file_list_t *search_file(int fileno)
{
    file_list_t *ret = NULL;
    for (int i=0; i<FILE_LIST_SIZE; i++) {
        if (file_list[i].fileno == fileno) ret = &file_list[i];
    }
    return ret;
}

int all_arrived()
{
    int ret=1;
    for (int i=0; i<70; i++) {
        //fprintf(stderr, "%d ", current->arrived[i]);
        ret*=current->arrived[i];
    }
    //fprintf(stderr, "ret %d\n", ret);
    return ret;
}

int send_nack(struct sockaddr_in *fromaddr, socklen_t *addrsize)
{
    int num=0, cnt=0;
    robust_nack_t msg;
    //fprintf(stderr, "send nack\n");
    for (int i=0; i<FILE_LIST_SIZE; i++) {
        if (file_list[i].stat == STAT_CURRENT) {
            int j=70;
            while (!file_list[i].arrived[j-1] && j>0) j--;
            while (j>0) {
                if (!file_list[i].arrived[j-1]) {
                    msg.msg.data[num++] = file_list[i].fileno*70 + (j-1);
                }
                j--;
            }
        } else if (file_list[i].stat == STAT_OLD) {
            int j=70;
            while (j>0) {
                if (!file_list[i].arrived[j-1]) {
                    msg.msg.data[num++] = file_list[i].fileno*70 + (j-1);
                }
                j--;
            }
        }
    }
    if (num > 0) {
        fprintf(stderr, "resend offer ");
        for (int i=0; i<num; i++) {
            fprintf(stderr, "%d ", msg.msg.data[i]);
            msg.msg.data[i] = htons(msg.msg.data[i]);
        }
        fprintf(stderr, "\n");
        /**
        **/
        if ((cnt=sendto(sfd, msg.data, num*2, 0, (struct sockaddr *)fromaddr, *addrsize)) < 0) {
            perror("sendto");
            return -1;
        }
    }
    return cnt;
}

void alarm_config()
{
    struct itimerval itimer;
    itimer.it_interval.tv_sec = 0;
    itimer.it_interval.tv_usec = 100;
    itimer.it_value = itimer.it_interval;
    if (setitimer(ITIMER_REAL, &itimer, NULL) < 0) {
        perror("setitimer");
        exit(-1);
    }
}