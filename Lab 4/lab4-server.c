// call using rempsd

//create and set up listening tcp socket
//go into an infinite loop accepting connections from clients
//for each connection, call handle_client()

#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#define port 3333
#define secret CSE3183

void handle_client(int connect_fd);

int main(int argc, char *argv[])
{
    char msg[201];
    int listen_fd, connect_fd, nread;
    struct sockaddr_in servaddr;

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int i = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

    if (listen_fd == -1)
    {
        printf("Socket failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Socket success\n");

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("Binding failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Bind success\n");

    if (listen(listen_fd, 10) != 0)
    {
        printf("Listen failed\n");
        exit(EXIT_FAILURE);
    }
    else
        printf("Listen success\n");

    while (1)
    {
        printf("Starting\n");
        connect_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL);
        if (connect_fd < 0)
        {
            printf("Accept failed\n");
            exit(EXIT_FAILURE);
        }

        nread = read(connect_fd, msg, 200);
        msg[nread] = '\0';
        printf("%s", msg);

        close(connect_fd);
    }
    return EXIT_SUCCESS;
}
