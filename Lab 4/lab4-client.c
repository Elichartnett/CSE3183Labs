// call using remps SERVER_IP_ADDRESS [user | cpu | mem]

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

    char remps[8];
    int nread;
    if ((nread = read(sock_fd, remps, 7)) < 0) //Protocol: Read <remps>
    {
        perror("Reading <remps> failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        remps[nread] = '\0';
        if (strcmp("<remps>", remps) != 0)
        {
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (write(sock_fd, secret, strlen(secret)) < 0) //Protocol: Write <shared secred>
    {
        perror("Writing <shared secret> failed\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    char ready[8];
    if ((nread = read(sock_fd, ready, 7)) < 0) //Protocol: Read <ready>
    {
        perror("Reading <ready> failed\n");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        ready[nread] = '\0';
        if (strcmp("<ready>", ready) != 0)
        {
            close(sock_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (style == 1) // Send user ps command
    {
        char user[56] = "<user";
        int uid = getuid();
        char *user_name = getpwuid(uid)->pw_name;
        strcat(user, user_name);
        strcat(user, ">");

        if (write(sock_fd, user, strlen(user)) < 0)
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

    char output[6]; //Could tweak for efficiency
    while ((nread = read(sock_fd, output, 5)) > 0) //Read arbitrary amount of text (Block until text received, then set to nonblocking so client ends after recived ps command output)
    {
        output[nread] = '\0';
        printf("%s", output);
        fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL) | O_NONBLOCK);
    }

    close(sock_fd);
    return EXIT_SUCCESS;
}
