#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>

#define PORT 3490
#define SIZE 1024

void handle_client(int client_fd) {
    char buffer[SIZE];
    int bytes;

    while (1) {
        memset(buffer, 0, SIZE);
        bytes = recv(client_fd, buffer, SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes] = '\0';
        printf("Client says: %s", buffer);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        printf("Server reply: ");
        fgets(buffer, SIZE, stdin);

        if (send(client_fd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0)//בודק אם 4 תווים ראשונים של ההודעה שווים ליציאה
            break;
    }

    close(client_fd);
    exit(0);
}


int main(void) {

    createSocket();
    fillHints();
    Bind();
    startListen();
    handleConnection();

    //
    int sockfd, new_fd;
    struct sockaddr_in my_addr, their_addr;
    socklen_t sin_size;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }
    //
    
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;//השרת מקשיב על כל כתובות ip של המחשב
    memset(&(my_addr.sin_zero), '\0', 8);
    
    //
    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    if (listen(sockfd, 4) == -1) {
        perror("listen");
        close(sockfd);
        exit(1);
    }

    printf("Server: waiting for a connection on port %d...\n", PORT);

    while (1) {
        sin_size = sizeof(struct sockaddr_in);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        printf("New client connected.\n");

        if (!fork()) {
            close(sockfd);
            handle_client(new_fd);
        }

        close(new_fd);
    }

    return 0;
}