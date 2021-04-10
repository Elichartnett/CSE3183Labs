// gcc -Wall -o client lab4-client.c
// ./client 127.0.0.1

// call using remps SERVER_IP_ADDRESS [user | cpu | mem]
// NEEDS TO ALSO HANDLE OPTION OF WHERE PS COMMAND RUNS ON SERVER
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>

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
    }
    else
    {
        printf("Usage: remps SERVER_IP_ADDRESS [user | cpu | mem]\n");
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    struct sockaddr_in servaddr;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); //Creating socket for server //ERROR CHECK

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    connect(sock_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)); //Connecting to server //ERROR CHECK

    char remps[8];
    int nread;
    if ((nread = read(sock_fd, remps, 7)) < 0) //Step 2 of protocol: Read <remps>
    {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        remps[nread] = '\0';
        if (strcmp("<remps>", remps) != 0)
        {
            printf("2. FAILED\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("2. Received <remps>\n");
        }
    }

    if (write(sock_fd, secret, strlen(secret)) < 0) //Step 3 of protocol: Write <shared secred>
    {
        perror("Write failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("3. Writing <shared secret>\n");
    }

    char ready[8];
    if ((nread = read(sock_fd, ready, 7)) < 0) //Step 6 of protocol: Read <ready>
    {
        perror("Reading failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        ready[nread] = '\0';
        if (strcmp("<ready>", ready) != 0)
        {
            printf("6. FAILED\n");
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("6. Received <ready>\n");
        }
    }

    if (style == 1) //Step 7: Send user ps command
    {
        char user[56] = "<user>";
        int uid = getuid();
        char *user_name = getpwuid(uid)->pw_name;
        strcat(user, user_name);

        if (write(sock_fd, user, strlen(user)) < 0)
        {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
        else
            printf("7. Writing %s\n", user);
    }
    else if (style == 2) //Step 7: Send cpu ps command
    {
        if (write(sock_fd, "<cpu>", 5) < 0)
        {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
        else
            printf("7. Writing <cpu>\n");
    }
    else if (style == 3) //Step 7: Send mem ps command
    {
        if (write(sock_fd, "<mem>", 5) < 0)
        {
            perror("Write failed");
            exit(EXIT_FAILURE);
        }
        else
            printf("7. Writing <mem>\n");
    }
    else
    {
        printf("Invalid style\n");
        exit(EXIT_FAILURE);
    }

    char *output = malloc(100);
    if ((nread = read(sock_fd, output, 99)) < 0) //Step 9 of protocol: Print ps command output
    {
        perror("Reading failed");
        exit(EXIT_FAILURE);
    }

    else
    {
        output[nread] = '\0';
        printf("10. Output:\n");
        printf("%s\n", output);
    }

    close(sock_fd);

    return EXIT_SUCCESS;
}
