#ifndef SRVPOLL_H
#define SRVPOLL_H

#include "parse.h"
#include <poll.h>

#define MAX_CLIENTS 256

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
    STATE_HELLO,
    STATE_MSG,
    STATE_GOODBYE,
} state_e;

// Structure to hold client state
typedef struct {
    int fd;
    state_e state;
    char buffer[4096];
} clientstate_t;

void init_clients(clientstate_t *clientStates);
int find_free_slot(clientstate_t *clientStates);
int find_slot_by_fd(clientstate_t *clientStates, int fd);

void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t **employees, clientstate_t *client);

#endif // SRVPOLL_H

