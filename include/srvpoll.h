#ifndef SRVPOLL_H
#define SRVPOLL_H

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

#endif // SRVPOLL_H

