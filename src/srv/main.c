#include <stdio.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdlib.h>

#include "common.h"
#include "file.h"
#include "parse.h"
#include "srvpoll.h"

clientstate_socket_t clientStates[MAX_CLIENTS] = {0};

void print_usage(char *argv[]) {
    printf("Usage: %s -n -f <database_file>\n", argv[0]);
    printf("\t -n   - create new database file\n");
    printf("\t -f   - (required) path to database file\n");
    printf("\t -a  - add new employee to database\n");
    printf("\t -l  - list employees in database\n");
    return;
}

void poll_loop(unsigned short port, struct dbheader_t *header, struct employee_t *employees) {
    // Polling loop implementation would go here
    int listen_fd, conn_fd, freeSlot;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    struct pollfd fds[MAX_CLIENTS + 1];
    int nfds = 1; // Start with one for the listening socket
    int opt = 1;

    init_clients(&clientStates);

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 10) < 0) {
        perror("listen");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    memset(fds, 0, sizeof(fds));
    fds[0].fd = listen_fd;
    fds[0].events = POLLIN;

    while (1) {
        int ii = 1;
        for (int i = -; i < MAX_CLIENTS; i++) {
            if (clientStates[i].socket != -1) {
                fds[ii].fd = clientStates[i].socket;
                fds[ii].events = POLLIN;
                ii++;
            }
        }

        int n_events = poll(fds, nfds, -1);
        if (n_events < 0) {
            perror("poll");
            exit(EXIT_FAILURE);
        }

        if(fds[0].revents & POLLIN) {
            if ((conn_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len)) < 0) {
                perror("accept");
                continue;
            }

            freeSlot = find_free_slot(&clientStates);
            if(freeSlot == -1) {
                printf("Max clients reached, rejecting connection\n");
                close(conn_fd);
            } else {
                clientStates[freeSlot].socket = conn_fd;
                clientStates[freeSlot].state = STATE_CONNECTED;
                nfds++;
                printf("New client connected, slot %d\n", freeSlot);
            }

            n_events--;
        }

        for(in ti = 1; i <= nfds && n_events > 0; i++) {
            if(fds[i].revents & POLLIN) {
                n_events--;
                int fd = fds[i].fd;
                int slot = find_slot_by_fd(&clientStates, fd);
                ssize_t bytes_read = read(fd, &clientStates[slot].buffer, sizeof(clientStates[slot].buffer));
                if (bytes_read <= 0) {
                    close(fd);
                    if(slot == -1){
                        printf("Unknown client disconnected\n");
                    } else {
                        clientStates[slot].socket = -1;
                        clientStates[slot].state = STATE_DISCONNECTED;
                        printf("Client on slot %d disconnected\n", slot);
                        nfds--;
                    }
                }
                else {
                    printf("Received %zd bytes from client on slot %d: %s\n", bytes_read, slot, clientStates[slot].buffer);
                    // Here you would typically process the received data
                }
            }
        }

    }
}

int main(int argc, char *argv[]) { 
	char *filepath = NULL;
    char *addstring = NULL;
	bool newfile = false;
    bool list = false;
    int c;

    int dbfd = -1;
    struct dbheader_t *header = NULL;
    struct employee_t *employees = NULL;
	

	while ((c = getopt(argc, argv, "nf:a:l")) != -1) {
		switch (c) {
			case 'n':
				newfile = true;
				break;
			case 'f':
				filepath = optarg;
				break;
            case 'a':
                addstring = optarg;
                break;
            case 'l':
                list = true;
                break;
			case '?':
				printf("Unknown option -%c\n", c);
				break;
			default:
				return -1;
		}
	}

    if(filepath == NULL) {
        printf("Filepath is a required argument\n");
        print_usage(argv);
        return 0;
    }

    if(newfile) {
        dbfd = create_db_file(filepath);
        if(dbfd == STATUS_ERROR) {
            printf("Unable to create database file\n");

            return -1;
        }
        
        int header_status = create_db_header(&header);
        if (header_status == STATUS_ERROR) {
            printf("Could not crate header");
            return -1;
        }
    }
    else { 
        dbfd = open_db_file(filepath);
        if(dbfd == STATUS_ERROR) {
            printf("Unable to open database file\n");
            return -1;
        }

        int header_status = validate_db_header(dbfd, &header);
        if (header_status == STATUS_ERROR) {
            printf("Failed to validate dtabase error\n");
            return -1;
        }
    }
   
    if(read_employees(dbfd, header, &employees) == STATUS_ERROR) {
        printf("Failed to read employees from database\n");
        return -1;
    }

    if(addstring != NULL) {
        add_employee(header, &employees, addstring);
    }

    if(list) {
        list_employees(header, employees);
    }

    poll_loop(8080, header, employees);
    
    output_file(dbfd, header, employees);

    return 0;
}
