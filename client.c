#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 3490
#define SIZE 1024

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in their_addr;
    char buffer[SIZE];
    int bytes;

    if (argc != 2) {//בדיקת כמות הארגומנטים שהתכנית לקוח מקבלת
        fprintf(stderr, "usage: %s <server-ip-address>\n", argv[0]);
        exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    their_addr.sin_family = AF_INET;
    their_addr.sin_port = htons(PORT);
    memset(&(their_addr.sin_zero), '\0', 8);

    if (inet_pton(AF_INET, argv[1], &their_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr *)&their_addr, sizeof(struct sockaddr)) == -1) {
        perror("connect");
        close(sockfd);
        exit(1);
    }

    printf("Connent to server at %s:%d\n", argv[1], PORT);

    while (1) {
        printf("Client: ");
        fgets(buffer, SIZE, stdin);

        if (send(sockfd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        memset(buffer, 0, SIZE);
        bytes = recv(sockfd, buffer, SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        buffer[bytes] = '\0';
        printf("Server: %s", buffer);

        if (strncmp(buffer, "exit", 4) == 0)
            break;
    }

    close(sockfd);
    return 0;
}
