#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "common.h"

void send_hello(int fd) {
    char buf[4096] = {0};

    dbproto_hdr_t *hdr = (dbproto_hdr_t *)buf;
    hdr->type = MSG_HELLO_REQ;
    hdr->len = 1;

    hdr->type = htons(hdr->type);
    hdr->len = htons(hdr->len);

    dbproto_hello_req *hello = (dbproto_hello_req *)(buf+sizeof(dbproto_hdr_t));
    hello->proto = htons(PROTO_VER);

    if(write(fd, buf, sizeof(dbproto_hdr_t) + sizeof(dbproto_hello_req)) > 0) {
        printf("HELLO header sent\n");
    } else {
        perror("write");
        return;
    }

}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <ip of the host>\n", argv[0]);
        return 0;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if(fd < 0) {
        perror("socket");
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(fd);
        return -1;
    }

    printf("Connected to server %s:%d\n", argv[1], SERVER_PORT);

    send_hello(fd);

    printf("Sent HELLO message to server\n");

    close(fd);
    return 0;
}