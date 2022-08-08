#include "sender.h"

int main()
{
    sequence = 0;
	skt_config();
	//init_list();
	file_buf_t buf;
    read_dir_file(sdata_dir);

	freeaddrinfo(dst_info);
	close(sfd);
	exit(0);
}

void skt_config()
{
	int err;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_DGRAM;
	if ((err = getaddrinfo(DST_ADDR, DST_PORT, &hints, &dst_info)) < 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(err));
		exit(-1);
	}

	if ((sfd=socket(dst_info->ai_family, dst_info->ai_socktype, dst_info->ai_protocol)) < 0) {
		perror("socket");
		exit(-1);
	}
}

int send_msg(robust_message_t *msg)
{
    int length = msg->msg.length;
    msg->msg.sequence = htons(sequence++);
    msg->msg.length = htons(msg->msg.length);
	return send_buf(msg->data, length+HEADER_SIZE);
}

int send_buf(uint8_t *buf, int length)
{
	int cnt;
	if ((cnt=sendto(sfd, buf, length, 0, dst_info->ai_addr, dst_info->ai_addrlen)) < 0) {
		perror("sendto");
		return -1;
	}
	return cnt;
}

int store_file(char *filename, file_buf_t *buf)
{
	struct stat st;
	int fd;
	int pos=0, readlen=0;
	
	if((fd = open(filename, O_RDONLY)) < 0) { return -1; }

	if (fstat(fd, &st) < 0) {
		perror("fstat");
		return -1;
	}
	while(pos < st.st_size) {
		if ((readlen=read(fd, buf->buf, sizeof(buf->buf))) < 0) {
			perror("read");
			return -1;
		}
		pos += readlen;
	}
	buf->size = pos;
	buf->pos = 0;
	close(fd);
	return 0;
}

int send_file(file_buf_t *buf)
{
    int pos = 0;
    history_list_t *history;
    robust_message_t *msg;
    while ((buf->size - DATA_MAX) > pos) {
        history = old_history(sequence);
        history->sequence = sequence;
        msg = &history->msg;
        memset(msg->data, 0, sizeof(msg->data));
        msg->msg.length = DATA_MAX;
        memcpy(msg->msg.data, &buf->buf[pos], DATA_MAX);
        if (send_msg(msg) < 0) {
            fprintf(stderr, "## Error on sending message ##\n");
            return -1;
        }
        pos += DATA_MAX;
    }
    if ((buf->size - pos) > 0) {
        history = old_history(sequence);
        history->sequence = sequence;
        msg = &history->msg;
        memset(msg->data, 0, sizeof(msg->data));
        msg->msg.length = buf->size - pos;
        memcpy(msg->msg.data, &buf->buf[pos], (buf->size - pos));
        if (send_msg(msg) < 0) {
            fprintf(stderr, "## Error on sending message ##\n");
            return -1;
        }
    }
    return 0;
}

void read_dir_file(char *dir_name) {
	char file_path[PATH_MAX];
	file_buf_t buf;
	for(int file_count = 0; file_count <= 10; file_count++) {
		snprintf(file_path, sizeof(file_path), "%s%s%d", dir_name, file_name_prefix, file_count);
		printf("file_path: %s\n", file_path);

        memset(&buf, 0, sizeof(buf));
		//store data on memory
		if (store_file(file_path, &buf) < 0) continue;
		//send data
        if (send_file(&buf) < 0) continue;
        usleep(1000);
	}
}

void init_list()
{
    for (int i=0; i<HISTORY_HASH_SIZE; i++) {
        for (int j=0; j<HISTORY_LIST_SIZE; j++) {
            history_list[i][j].sequence = -1;
        }
    }
}

static inline int gen_hash(int seq) { return seq % HISTORY_HASH_SIZE; }
history_list_t *old_history(int seq)
{
    int hash = gen_hash(seq);
    static int index[HISTORY_HASH_SIZE];
    history_list_t *ret = &history_list[hash][index[hash]];
    index[hash] = (index[hash]+1) % HISTORY_LIST_SIZE;
    return ret;
}

history_list_t *search_history(int seq)
{
    history_list_t *ret = NULL;
    int hash = gen_hash(seq);
    for (int i=0; i<HISTORY_LIST_SIZE; i++) {
        if (history_list[hash][i].sequence == seq) ret = &history_list[hash][i];
    }
    return ret;
}