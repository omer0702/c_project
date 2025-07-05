#ifndef SERVER_H
#define SERVER_H

#include <pthread.h>
#include <netinet/in.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

extern int clients[MAX_CLIENTS];
extern char client_names[MAX_CLIENTS][30];
extern int client_count;
extern pthread_mutex_t lock;

void broadcast(char* msg, int sender_fd);
void remove_client(int client_fd);
void* handle_client(void* arg);

#endif
