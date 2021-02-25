// Team: Eli Hartnett (ech364) and Tara Broome (tb2120)
// compile: gcc -Wall -std=c99 -o lab2 lab2.c
// valid calls: ./lab1 [style] <filename(s)> or ./lab1 [style] or ./lab1 <filename(s)> or ./lab1

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#define MAX_LINE_LENGTH 100

// Prototypes:
void process_stream(int fd, char bstylechar, char *sstyle);
char *fgetline(int fd);
int fgetchar(int fd);

int main(int argc, char *argv[])
{
    char bstylechar = 't';
    char *sstyle = "\t";
    int firstfile = 1;
    int option = 0;

    while ((option = getopt(argc, argv, "b:s:")) != -1)
    {
        switch (option)
        {
        case 'b':
            if (strcmp(optarg, "a") == 0)
            {
                bstylechar = 'a';
            }
            else if (strcmp(optarg, "n") == 0)
            {
                bstylechar = 'n';
            }
            else if (strcmp(optarg, "t") == 0)
            {
                bstylechar = 't';
            }
            else
            {
                printf("%s: invalid body numbering style: '%s'\n", argv[0], optarg);
                return EXIT_FAILURE;
            }
            break;

        case 's':
            sstyle = optarg;
            break;

        case '?':
            return EXIT_FAILURE;
        }
        firstfile = optind;
    }

    bool error = false;
    if (argc > firstfile) // file(s) supplied + maybe b option
    {
        for (int i = firstfile; i < argc; i++)
        {
            int fd;
            if ((fd = open(argv[i], O_RDONLY)) == -1)
            {
                fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
                error = true;
            }
            else
            {
                process_stream(fd, bstylechar, sstyle);
                close(fd);
            }
        }
    }

    else //handle no file(s)
    {
        process_stream(0, bstylechar, sstyle);
    }

    if (error)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

void process_stream(int fd, char bstylechar, char *sstyle)
{
    char *line;
    static int count = 1;
    int spacer = 6 + strlen("\t"); // gets correct nl spacing when not numbering a line

    while ((line = fgetline(fd)) != NULL)
    {
        switch (bstylechar)
        {
        case 'a':
            printf("%6d\t%s%s\n", count++, sstyle, line);
            break;
        case 'n':
            printf("%*s%s%s\n", spacer, " ", sstyle, line);
            break;
        case 't':
            if (strcmp(line, "") == 0) // looks for empty lines
            {
                printf("%*s\n", spacer, "");
            }
            else
                printf("%6d\t%s%s\n", count++, sstyle, line);
            break;
        default:
            fprintf(stderr, "Error: invalid value specified\n");
            exit(EXIT_FAILURE);
        }
    }
}

char *fgetline(int fd)
{
    static char buff[MAX_LINE_LENGTH + 1];

    int next;
    int pos = 0;
    while ((next = fgetchar(fd)) != '\n' && next != EOF)
    {
        buff[pos++] = next;
    }

    buff[pos] = '\0'; // makes buff a valid C string of correct length

    if (next == EOF && pos == 0) // handles non-newline terminated files
    {
        return NULL;
    }
    return buff;
}

int fgetchar(int fd)
{
    int letter = 0;
    int fd_read = read(fd, &letter, 1);

    if (fd_read == 0)
        return EOF;

    if (fd_read == -1)
    {
        printf("Error reading file.");
    }

    return letter;
}
