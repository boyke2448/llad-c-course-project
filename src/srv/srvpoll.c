#include "srvpoll.h"


void init_clients(clientstate_socket_t *clientStates) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clientStates[i].socket = -1;
        clientStates[i].state = STATE_NEW;
        memset(clientStates[i].buffer, 0, sizeof(clientStates[i].buffer));
    }
}


int find_free_slot(clientstate_socket_t *clientStates)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientStates[i].socket == -1)
        {
            return i;
        }
    }
    return -1;
}

int find_slot_by_fd(clientstate_socket_t *clientStates, int fd)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clientStates[i].socket == fd)
        {
            return i;
        }
    }
    return -1;
}