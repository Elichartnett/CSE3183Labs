// call using ./remps SERVER_IP_ADDRESS [user | cpu | mem]

#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <fcntl.h>

#define port 3333
#define secret "CSE3183"

const char * const msg_remps = "<remps>";
const char * const msg_ready = "<ready>";
const char * const msg_invalid = "<invalid>";
const char * const msg_secret = "<" secret ">";

int main(int argc, char *argv[])
{
    char *server_ip = argv[1];

    int style;
    if (argc == 2) //Default option (user)
        style = 1;
    else if (argc == 3)
    {
        if (strcmp(argv[2], "user") == 0) //Option 1: user
            style = 1;
        else if (strcmp(argv[2], "cpu") == 0) //Option 2: cpu
            style = 2;
        else if (strcmp(argv[2], "mem") == 0) //Option 3: mem
            style = 3;
        else
        {
            printf("Usage: remps SERVER_IP_ADDRESS [user | cpu | mem]\n");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("Usage: remps SERVER_IP_ADDRESS [user | cpu | mem]\n");
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in servaddr;

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) //Creating socket for server
    {
        perror("socket() failed\n");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("connect() failed\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    char received_remps[strlen(msg_remps) + 1];
    int nread;
    if ((nread = read(sock_fd, received_remps, strlen(msg_remps))) < 0) //Protocol: Read <remps>
    {
        perror("Reading <remps> failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        received_remps[nread] = '\0';
        if (strcmp("<remps>", received_remps) != 0)
        {
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
    if (write(sock_fd, msg_secret, strlen(msg_secret)) < 0) //Protocol: Write <shared secred>
    {
        perror("Writing <shared secret> failed\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    char received_ready[strlen(msg_ready) + 1];
    if ((nread = read(sock_fd, received_ready, strlen(msg_ready))) < 0) //Protocol: Read <ready>
    {
        perror("Reading <ready> failed\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        received_ready[nread] = '\0';
        if (strcmp("<ready>", received_ready) != 0)
        {
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
    if (style == 1) // Send user ps command
    {
        char user_name[42] = "<user:";
        int uid = getuid();
        char *user = getpwuid(uid)->pw_name;
        strcat(user_name, user);
        strcat(user_name, ">");

        if (write(sock_fd, user_name, strlen(user_name)) < 0)
        {
            perror("Writing directive failed\n");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
    else if (style == 2) //Send cpu ps command
    {
        if (write(sock_fd, "<cpu>", 5) < 0)
        {
            perror("Writing directive failed\n");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
    else if (style == 3) //Send mem ps command
    {
        if (write(sock_fd, "<mem>", 5) < 0)
        {
            perror("Writing directive failed\n");
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }
    char output[1025];
    while ((nread = read(sock_fd, output, 1024)) > 0) //Read arbitrary amount of text 
    {
        output[nread] = '\0';
        printf("%s", output);
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
