// Call using ./rempsd

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

const char *const msg_remps = "<remps>";
const char *const msg_ready = "<ready>";
const char *const msg_invalid = "<invalid>";
const char *const msg_secret = "<" secret ">";

void handle_client(int connect_fd);

int main(int argc, char *argv[])
{
    if (argc == 1)
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
            {
                perror("setsockopt() failed");
                exit(EXIT_FAILURE);
            }
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
                dup2(connect_fd, 2);
                handle_client(connect_fd);
                close(connect_fd);
            }
        }
        return EXIT_FAILURE; //Failure because this should not be reached
    }
    else
    {
        printf("Usage: ./rempsd\n");
        exit(EXIT_FAILURE);
    }
}

void handle_client(int connect_fd)
{
    if (write(connect_fd, msg_remps, strlen(msg_remps)) <= 0) //Protocol: Send <remps>
    {
        perror("Writing msg_remps failed\n");
        close(connect_fd);
        return;
    }

    int nread;
    char received_secret[strlen(msg_secret) + 1];
    if ((nread = read(connect_fd, received_secret, strlen(msg_secret))) <= 0) //Protocol: Verify <secret>
    {
        perror("Reading msg_secret failed\n");
        return;
    }
    else
    {
        received_secret[nread] = '\0';
        if (strcmp(received_secret, msg_secret) != 0)
            return;
    }

    if (write(connect_fd, msg_ready, strlen(msg_ready)) < 0) //Protocol: Send <ready>
    {
        perror("Write <ready> failed\n");
        return;
    }
    char *directive = malloc(43);                      //Linux useradd man page sets max username length to 35; max directive is length 6 (<user:>); 35 + 7 + null char = 43
    if ((nread = read(connect_fd, directive, 42)) < 0) //Protocol: Get directive (user, cpu, or mem)
    {
        perror("Reading directive (user, cpu, or mem) failed\n");
        return;
    }
    else
    {
        directive[nread] = '\0';

        if ((strncmp(directive, "<user:", 6)) == 0) //Directive is user
        {
            char *user_name = directive + 6;
            user_name[strlen(user_name) - 1] = '\0';
            struct passwd *user_check = getpwnam(user_name);
            if (user_check == NULL) //Invalid username
                return;
            else
            {
                char *command = malloc(75); //Max size is 32 (username) + 33 (length of ps call)
                strcat(command, "ps -u ");
                strcat(command, user_name);
                strcat(command, " -o pid,ppid,%cpu,%mem,args");
                system(command);
                free(command);
            }
        }
        else if ((strcmp(directive, "<cpu>")) == 0) //Directive is cpu
        {
            char *command = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%cpu | head";
            system(command);
        }
        else if ((strcmp(directive, "<mem>")) == 0) //Directive is mem
        {
            char *command = "ps -NT -o pid,ppid,%cpu,%mem,args --sort -%mem | head";
            system(command);
        }
    }
    free(directive);
}
