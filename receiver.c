#include "receiver.h"

int main()
{
    skt_config();
    signal_config();
    while(running) {
        get_file();
    }
    fprintf(stderr, "Exiting...\n");
    close(sfd);
}

int get_file()
{
    robust_message_t msg;
    file_buf_t file;
    int cnt;
    memset(&file, 0, sizeof(file));
    while (running) {
        if ((cnt=get_msg(&fromaddr, &addrsize, &msg)) < 0) continue;
        file.fileno = msg.msg.sequence / 70;
        printf("%d %d\n", file.fileno, (msg.msg.sequence % 70));
        file.size += msg.msg.length;
        memcpy(&file.buf[(msg.msg.sequence % 70)*DATA_MAX], msg.msg.data, msg.msg.length);
        if (msg.msg.sequence % 69 == 0) break;
    }
    if (file.size == 0) return -1;
    return save_file(&file);
}

int save_file(file_buf_t *file)
{
    char file_path[PATH_MAX];
    memset(&file_path, 0, sizeof(file_path));
    snprintf(file_path, sizeof(file_path), "recv/%s%d", file_name_prefix, (*file).fileno);
    FILE *fd;
    if((fd = fopen(file_path, "w")) < 0) {
        perror("fopen");
        fclose(fd);
        return -1;
    }
    fwrite((*file).buf, 1, (*file).size, fd);
    fclose(fd);
    //fprintf(stderr, "%d\n", (*file).size);
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

void sigint_handler()
{
    running = 0;
}

void signal_config()
{
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    if (sigaction(SIGINT, &sa, NULL) < 0) {
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
        perror("recvfrom");
        return cnt;
    }
    if (cnt == 0) return -1;
    /**
    char fromaddrstr[16];
    if (inet_ntop(AF_INET, &fromaddr->sin_addr, fromaddrstr, sizeof(fromaddrstr)) < 0) {
        perror("inet_ntop");
        return -1;
    }
    **/
    memcpy(msg->data, buf, cnt);
    msg->msg.length = ntohs(msg->msg.length);
    msg->msg.sequence = ntohs(msg->msg.sequence);

    return cnt;
}