#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 3490
#define SIZE 1024

int main(void) {
    int sockfd, new_fd;
    struct sockaddr_in my_addr, their_addr;
    socklen_t sin_size;
    char buffer[SIZE];
    int bytes;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(1);
    }

    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT);
    my_addr.sin_addr.s_addr = INADDR_ANY;//השרת מקשיב על כל כתובות ip של המחשב
    memset(&(my_addr.sin_zero), '\0', 8);

    if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(1);
    }

    printf("Server: waiting for a connection on port %d...\n", PORT);

    sin_size = sizeof(struct sockaddr_in);
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
        perror("accept");
        exit(1);
    }

    printf("Client connected!\n");

    while (1) {
        memset(buffer, 0, SIZE);
        bytes = recv(new_fd, buffer, SIZE - 1, 0);
        if (bytes <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[bytes] = '\0';
        printf("Client: %s", buffer);

        if (strncmp(buffer, "exit", 4) == 0)
            break;

        printf("Server: ");
        fgets(buffer, SIZE, stdin);

        if (send(new_fd, buffer, strlen(buffer), 0) == -1) {
            perror("send");
            break;
        }

        if (strncmp(buffer, "exit", 4) == 0)//משווה 4 תווים של התשובה לexit
            break;
    }

    close(new_fd);
    close(sockfd);
    return 0;
}
