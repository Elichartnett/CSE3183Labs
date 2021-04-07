// call using remps SERVER_IP_ADDRESS [user | cpu | mem]
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#define port 3333
#define secret CSE3183


int main(int argc, char *argv[])
{

    char *server_ip = argv[1];
    char *msg = argv[2];

    int sock_fd;
    struct sockaddr_in servaddr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); //error check

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)); //error check

    printf("Writing\n");
    write(sock_fd, msg, strlen(msg));

    close(sock_fd);

    return EXIT_SUCCESS;
}