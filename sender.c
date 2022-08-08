#include "sender.h"

int main()
{
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
    msg->msg.fileno = htons(msg->msg.fileno);
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
    int pos = 0, seq = 0;
    robust_message_t msg;
    while ((buf->size - DATA_MAX) > pos) {
        memset(msg.data, 0, sizeof(msg.data));
        msg.msg.code = CODE_DATA;
        msg.msg.fileno = buf->fileno;
        msg.msg.length = DATA_MAX;
        msg.msg.sequence = seq;
        memcpy(msg.msg.data, &buf->buf[pos], DATA_MAX);
        if (send_msg(&msg) < 0) {
            fprintf(stderr, "## Error on sending message ##\n");
            return -1;
        }
        seq++;
        pos += DATA_MAX;
    }
    if ((buf->size - pos) > 0) {
        memset(msg.data, 0, sizeof(msg.data));
        msg.msg.code = CODE_DATA_LAST;
        msg.msg.fileno = buf->fileno;
        msg.msg.length = buf->size - pos;
        msg.msg.sequence = seq;
        memcpy(msg.msg.data, &buf->buf[pos], (buf->size - pos));
        if (send_msg(&msg) < 0) {
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
        buf.fileno = file_count;
		//store data on memory
		if (store_file(file_path, &buf) < 0) continue;
		//send data
        if (send_file(&buf) < 0) continue;
        //usleep(1000);
	}
}

void init_list()
{
	file_buf_head.fp = &file_buf_head;
	file_buf_head.bp = &file_buf_head;
	for (int i=0; i<10; i++) {
		file_buf_list[i].sent = 1;
		insert_head(&file_buf_list[i]);
	}
}

void insert_head(file_buf_list_t *buf)
{
	buf->fp = file_buf_head.fp;
	buf->bp = &file_buf_head;
	file_buf_head.fp->bp = buf;
	file_buf_head.fp = buf;
}

void remove_from_list(file_buf_list_t *buf)
{
	buf->bp->fp = buf->fp;
	buf->fp->bp = buf->bp;
}

int search_file()
{
	char *filename = "unti";
	file_buf_list_t *ptr;
	for (ptr=file_buf_head.bp; ptr!=&file_buf_head; ptr=ptr->bp) {
		printf("sent %d\n", ptr->sent);
		if (ptr->sent == 1) break;
	}
	if (store_file(filename, &ptr->buf) < 0) {
		fprintf(stderr, "Error on reading file\n");
	}
	remove_from_list(ptr);
	insert_head(ptr);
	return 0;
}