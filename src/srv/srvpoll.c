#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "srvpoll.h"
#include "common.h"

void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t **employees, clientstate_t *client) {
    dbproto_hdr_t *hdr = (dbproto_hdr_t *)client->buffer;

    hdr->type = ntohs(hdr->type);
    hdr->len = ntohs(hdr->len);

    switch (client->state)
    {
    case STATE_HELLO:
        if(hdr->type != MSG_HELLO_REQ || hdr->len != 1){
            printf("Invalid HELLO message\n");
            return;
        }

        dbproto_hello_req *hello = (dbproto_hello_req*)&hdr[1];
        hello->proto = ntohs(hello->proto);
        if (hello->proto != PROTO_VER){
            printf("Unsupported protocol version: %d\n", hello->proto);
            return;
        }
        printf("Client HELLO received, protocol version: %d\n", hello->proto);
        client->state = STATE_MSG;
        break;
    case STATE_MSG:
        break;
    default:
        break;
    }
}

void init_clients(clientstate_t *clientStates) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientStates[i].fd = -1;
        clientStates[i].state = STATE_NEW;
        memset(&clientStates[i].buffer, 0, sizeof(clientStates[i].buffer));
    }
}


int find_free_slot(clientstate_t *clientStates)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientStates[i].fd == -1)
        {
            return i;
        }
    }
    return -1;
}

int find_slot_by_fd(clientstate_t *clientStates, int fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientStates[i].fd == fd)
        {
            return i;
        }
    }
    return -1;
}