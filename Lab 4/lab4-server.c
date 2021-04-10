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
        perror("socket() failed\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        int i = 1;
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) < 0) //Setting socket option to reuse address
            perror("setsockopt() failed");
    }

    servaddr.sin_family = AF_INET; //Setting up server port and address
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) //Binding socket to port and address
    {
        perror("bind() failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(listen_fd, 10) != 0) //Listening for connections
    {
        perror("listen() failed");
        close(listen_fd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        connect_fd = accept(listen_fd, (struct sockaddr *)NULL, NULL); //Client connected
        if (connect_fd < 0)
        {
            perror("accept() failed");
            exit(EXIT_FAILURE);
        }
        else
        {
            dup2(connect_fd, 1);

            handle_client(connect_fd);
            close(connect_fd);
            printf("--------------------\n");
        }
    }
    return EXIT_SUCCESS;
}

void handle_client(int connect_fd)
{
    if (write(connect_fd, "<remps>", 7) < 0) //Protocol: Send <remps>
    {
        perror("Writing <remps> failed\n");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    int nread;
    char received_secret[strlen(secret) + 1];
    if ((nread = read(connect_fd, received_secret, strlen(secret))) < 0) //Protocol: Verify <shared secret>
    {
        perror("Reading <shared secret> failed\n");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        received_secret[nread] = '\0';
        if (strcmp(received_secret, secret) != 0)
        {
            printf("Incorrect shared secret\n");
            close(connect_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (write(connect_fd, "<ready>", 7) < 0) //Protocol: Send <ready>
    {
        perror("Write <ready> failed\n");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }

    char *directive = malloc(50); //NEED TO ADD DYNAMIC MEMORY
    if ((nread = read(connect_fd, directive, 56)) < 0) //Protocol: Get directive (user, cpu, or mem)
    {
        perror("Reading directive (user, cpu, or mem) failed\n");
        close(connect_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        directive[nread] = '\0';

        if ((strncmp(directive, "<user>", 6)) == 0) //Directive is user
        {
            char *user_name = directive + 6;
            struct passwd *user_check = getpwnam(user_name);
            if (user_check == NULL)
            {
                printf("Invalid username\n");
                close(connect_fd);
                exit(EXIT_FAILURE);
            }
            else
            {
                char *command = malloc(100); //NEED TO ADD MEMORY BASED ON DIRECTIVE LENGTH
                strcat(command, "ps -u ");
                strcat(command, user_name);
                strcat(command, " -o pid,ppid,%cpu,%mem,args");
                system(command);
                free(command);
            }
        }
        else if ((strcmp(directive, "<cpu>")) == 0) //Directive is cpu
        {
            printf("8. Received <cpu>:\n");

            printf("9. Sending output to client\n");
            dup2(connect_fd, 1);
            dup2(connect_fd, 2);
            char *command = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head";
            system(command);
        }
        else if ((strcmp(directive, "<mem>")) == 0) //Directive is mem
        {
            printf("8. Received <mem>:\n");

            printf("9. Sending output to client\n");
            dup2(connect_fd, 1);
            dup2(connect_fd, 2);
            char *command = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head";
            system(command);
        }
    }
    free(directive);
}
