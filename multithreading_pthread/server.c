#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "server.h"

#define PORT 8080
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];//מערך שמכיל סוקטי לקוחות
char client_names[MAX_CLIENTS][30];
int client_count = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;//מנעול לסנכרון בין תהליכונים

void broadcast(char* msg, int sender_fd) {//שליחת הודעה לכל הלקוחות חוץ מהשולח
    int i;
    pthread_mutex_lock(&lock);//נעילת גישה לנתונים המשותפים

    for (i = 0; i < client_count; i++) {
        if (clients[i] != sender_fd) {
            send(clients[i], msg, strlen(msg), 0);
        }
    }

    pthread_mutex_unlock(&lock);//שחרור המנעול
}

void remove_client(int client_fd) {
    int i;
    pthread_mutex_lock(&lock);

    for (i = 0; i < client_count; i++) {
        if (clients[i] == client_fd) {
            //הזזת כל שאר הלקוחות מקום אחורה כדי לא להשאיר חור במערך
            int j;

            for (j = i; j < client_count - 1; j++) {
                clients[j] = clients[j+1];
                strcpy(client_names[j], client_names[j+1]);
            }
            client_count--;
            break;
        }
    }

    pthread_mutex_unlock(&lock);
}

void* handle_client(void* arg) {//רצה בכל תהליכון עבור לקוח
    int client_fd = *(int*)arg;
    free(arg); // שחרור הזיכרון שהוקצה ב-main
    char buffer[BUFFER_SIZE];
    char name[30];

    if (recv(client_fd, name, sizeof(name), 0) <= 0) {//שם בname את שם הלקוח
        close(client_fd);
        return NULL;
    }

    //הוספת הלקוח החדש למערכים של הלקוחות תוך מניעת התנגשויות בין תהליכונים
    pthread_mutex_lock(&lock);
    clients[client_count] = client_fd;
    strcpy(client_names[client_count], name);
    client_count++;
    pthread_mutex_unlock(&lock);

    //שולח לכל הלקוחות שלקוח חדש הצטרף
    sprintf(buffer, "%s joined the chat\n", name);//מכניס את ההודעה לתוך הבאפר במקום להדפיס במסך
    broadcast(buffer, client_fd);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(client_fd, buffer, BUFFER_SIZE, 0);
        if(bytes <=0){
            break;
        }

        //שליחת השמות של כל המשתתפים
        if (!(strcmp(buffer, "/list\n"))) {
            int i;
            char list[1024] = "Connected users:\n";
            pthread_mutex_lock(&lock);

            for (i = 0; i < client_count; i++) {
                strcat(list, client_names[i]);//הוספת השם לרשימה
                //strcat(list, "\n");
            }

            pthread_mutex_unlock(&lock);
            send(client_fd, list, strlen(list), 0);
        }
        //אם זה הודעה רגילה אז נצרף את שם השולח ונשלח לשאר הלקוחות
        else {
            char msg[BUFFER_SIZE + 100];
            sprintf(msg, "%s: %s", name, buffer);
            broadcast(msg, client_fd);
        }
    }

    //הודעה לכולם שמישהו עזב
    sprintf(buffer, "%s left the chat\n", name);
    broadcast(buffer, client_fd);

    remove_client(client_fd);//הסרת הלקוח ממערך הלקוחות


    close(client_fd);
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pthread_t t_id;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, MAX_CLIENTS) == -1) {
        perror("listen failed");
        close(server_fd);
        exit(1);
    }

    printf("server listening on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &addr_len);

        if (client_fd == -1) {
            perror("accept failed");
            continue;
        }        
        
        int* new_client = malloc(sizeof(int));
        *new_client = client_fd;
        pthread_create(&t_id, NULL, handle_client, (void*)new_client);
        pthread_detach(t_id);//אומר שאין צורך לחכות שהתהליכון יסתיים
    }

    close(server_fd);
    return 0;
}