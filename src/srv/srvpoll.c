#include <string.h>
#include "srvpoll.h"

void handle_client_fsm(struct dbheader_t *dbhdr, struct employee_t **employeeptr, clientstate_t *client, int dbfd) {
    
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