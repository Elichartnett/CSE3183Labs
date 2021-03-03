// Name: Eli Hartnett (ech364)
// compile: gcc -Wall -std=c99 -o lab2 lab2.c
// valid calls: ./lab1 [styles] <filename(s)> or ./lab2 [styles] or ./lab2 <filename(s)> or ./lab2

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <fcntl.h>

#define FILE_BUFF_SIZE 10 //512
#define INIT_BUFF_SIZE 5  //50
#define INC_BUFF_SIZE 1   //10

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
    if (argc > firstfile) //handles option(s) and files
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
    int spacer = 6 + strlen("\t");

    while ((line = fgetline(fd)) != NULL)
    {
        switch (bstylechar) //output
        {
        case 'a':
            printf("%6d%s%s\n", count++, sstyle, line);
            break;
        case 'n':
            printf("%*s%s%s\n", spacer, " ", sstyle, line);
            break;
        case 't':
            if (strcmp(line, "") == 0)
            {
                printf("%*s\n", spacer, "");
            }
            else
                printf("%6d%s%s\n", count++, sstyle, line);
            break;
        }
    }
}

char *fgetline(int fd)
{
    char *line_buff = malloc(FILE_BUFF_SIZE + 1);
    size_t buff_size = FILE_BUFF_SIZE;
    int buff_pos = 0, next_char;

    while ((next_char = fgetchar(fd)) != '\n' && next_char != EOF)
    {
        if (buff_pos == buff_size)
        {
            if ((line_buff = realloc(line_buff, buff_size + INC_BUFF_SIZE)) != NULL)
                buff_size += INC_BUFF_SIZE;
            else
                exit(EXIT_FAILURE);
        }
        line_buff[buff_pos++] = next_char;
    }

    if (next_char == EOF && (buff_pos == 0 || errno))
    {
        free(line_buff);
        return NULL;
    }
    line_buff[buff_pos] = '\0';
    return line_buff;
}

int fgetchar(int fd)
{
    static char buff[FILE_BUFF_SIZE];
    static int chars_left = 0;
    static int pos;
    int nread;

    if (chars_left == 0)
    {
        if ((nread = read(fd, buff, FILE_BUFF_SIZE)) <= 0)
        {
            return EOF;
        }
        chars_left = nread;
        pos = 0;
    }

    chars_left--;
    return buff[pos++];
}