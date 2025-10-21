#ifndef SRVPOLL_H
#define SRVPOLL_H

#define MAX_CLIENTS 256

typedef enum {
    STATE_NEW,
    STATE_CONNECTED,
    STATE_DISCONNECTED,
} clientstate_enum_t;

typedef struct {
    int socket;
    char buffer[1024];
    clientstate_enum_t state;
} clientstate_socket_t;

void init_clients(clientstate_socket_t *clientStates);
int find_free_slot(clientstate_socket_t *clientStates);
int find_slot_by_fd(clientstate_socket_t *clientStates, int fd);

#endif // SRVPOLL_H

