#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "client.h"

#define PORT 8080
#define BUFFER_SIZE 1024//גודל מקסימלי להודעות

int sock;
int running = 1;

void* receive_handle(void* arg) {
    char buffer[BUFFER_SIZE];

    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes <= 0) {
            printf("disconnected from server.\n");
            running = 0;
            break;
        }
        printf("%s", buffer);
    }

    return NULL;
}


int main() {
    struct sockaddr_in server_addr;
    pthread_t t_id;
    char buffer[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket creation failed");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connection to server failed");
        exit(1);
    }

    //שליחת השם לשרת
    printf("enter your name: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = 0;//מחיקת ירידת השורה בשביל ההדפסה בטרמינל
    send(sock, buffer, strlen(buffer), 0);

    pthread_create(&t_id, NULL, receive_handle, NULL);

    //שליחת הודועת ריגלות לשרת, כרגע רק אם נשלח list יתבצע משהו
    while (running) {
        memset(buffer, 0, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);
        //buffer[strcspn(buffer, "\n")] = '\0';

        if (!(strncmp(buffer, "/exit", 5))) {
            running = 0;
            break;
        }

        if (send(sock, buffer, strlen(buffer), 0) <= 0) {
            printf("failed to send message, exiting\n");
            break;
        }
    }

    close(sock);
    printf("connection closed. bye!\n");

    return 0;
}
