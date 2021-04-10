// gcc -Wall -o server lab4-server.c
// ./server

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
#include <pwd.h>
#include <sys/wait.h>

#define port 3333
#define secret "CSE3183"

void handle_client(int connect_fd);

int main(int argc, char *argv[])
{
    int listen_fd, connect_fd;
    struct sockaddr_in servaddr;

    if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //Creating socket
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        int i = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0) //Setting socket option to reuse address
            perror("Setting sockopt failed");
    }

    servaddr.sin_family = AF_INET; //Setting up server port and address
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) //Binding socket to port and address
    {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 10) != 0) //Listening for connections
    {
        printf("Listen failed");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        connect_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL); //Client connected
        if (connect_fd < 0)
        {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            handle_client(connect_fd);
            printf("--------------------\n");
        }
    }
    return EXIT_SUCCESS;
}

void handle_client(int connect_fd)
{
    if (write(connect_fd, "<remps>", 7) < 0) //Step 1 of protocol: Send <remps>
    {
        perror("Write failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("1. Writing <remps>\n");
    }

    int nread;
    char received_secret[strlen(secret) + 1];
    if ((nread = read(connect_fd, received_secret, strlen(secret))) < 0) //Step 4 of protocol: Verify <shared secret>
    {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        received_secret[nread] = '\0';
        if (strcmp(received_secret, secret) == 0)
        {
            printf("4. Recived <shared secret>\n");
        }
        else
        {
            printf("4. FAILED\n");
            exit(EXIT_FAILURE);
        }
    }

    if (write(connect_fd, "<ready>", 7) < 0) //Step 5 of protocol: Send <ready>
    {
        perror("Write failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("5. Writing <ready>\n");
    }

    char *directive = malloc(50);
    if ((nread = read(connect_fd, directive, 56)) < 0) //Step 8 of protocol: Get directive (user, cpu, or mem) //NOT SURE WHAT MAX NAME CAN BE
    {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        directive[nread] = '\0';

        if ((strncmp(directive, "<user>", 6)) == 0) //Directive is user
        {

            struct passwd *user_check = getpwnam(directive + 6);
            if (user_check == NULL)
            {
                printf("8. FAILED\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("8. Received <user>%s\n", directive + 6);

                printf("9. Sending output to client\n");
                //dup2(connect_fd, 1);
                //dup2(connect_fd, 2);
                char *command = malloc(100);
                command = "ps -u ";
                strcat(command, directive + 6);
                strcat(command, " -o pid ppid %cpu %mem args");
                printf("%s\n", command);
            }
        }
        else if ((strcmp(directive, "<cpu>")) == 0) //Directive is cpu
        {
            printf("8. Received <cpu>:\n");

            printf("9. Sending output to client\n");
            dup2(connect_fd, 1);
            dup2(connect_fd, 2);
            char *command = "ps -NT -o pid ppid %cpu %mem args --sort -%cpu | head";
            system(command);
        }
        else if ((strcmp(directive, "<mem>")) == 0) //Directive is mem
        {
            printf("8. Received <mem>:\n");

            printf("9. Sending output to client\n");
            dup2(connect_fd, 1);
            dup2(connect_fd, 2);
            char *command = "ps -NT -o pid ppid %cpu %mem args --sort -%mem | head";
            system(command);
        }
    }

    free(directive);
    close(connect_fd);
}
