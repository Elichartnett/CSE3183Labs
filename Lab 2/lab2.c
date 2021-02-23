// Team: Eli Hartnett (ech364) and Tara Broome (tb2120)
// compile: gcc -Wall -std=c99 -o lab2 lab2.c
// valid calls: ./lab1 [style] <filename(s)> or ./lab1 [style] or ./lab1 <filename(s)> or ./lab1

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_LINE_LENGTH 100

// Prototypes:
void process_stream(FILE *fpntr, char bstylechar);
char *fgetline(FILE *fpntr);

int main(int argc, char *argv[])
{
    char bstylechar = 't';
    char sstylechar = '\t';
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
            sstylechar = *optarg;
            break;

        case '?':
            return EXIT_FAILURE;
        }
    }

    if (argc > firstfile) // file(s) supplied + maybe b option
    {
        for (int i = firstfile; i < argc; i++)
        {
            FILE *fpntr;
            if ((fpntr = fopen(argv[i], "r")) == NULL)
                fprintf(stderr, "%s: %s: %s\n", argv[0], argv[i], strerror(errno));
            else
            {
                process_stream(fpntr, bstylechar);
                fclose(fpntr);
            }
        }
    }

    else //handle no file(s)
    {
        process_stream(stdin, bstylechar);
    }

    return EXIT_SUCCESS;
}

void process_stream(FILE *fpntr, char bstylechar)
{
    char *line;
    static int count = 1;
    char *spacer = "\t"; // gets correct nl spacing when not numbering a line

    while ((line = fgetline(fpntr)) != NULL)
    {
        if (strcmp(line, "\0") != 0)
        {
            switch (bstylechar)
            {
            case 'a':
                printf("%6d\t%s\n", count, line);
                break;
            case 'n':
                printf("%s%s\n", spacer, line);
                break;
            case 't':
                if (strcmp(line, "\n") == 0) // looks for empty lines
                {
                    printf("\n");
                    count--;
                }
                else
                    printf("%6d\t%s\n", count, line);
                break;
            }
            count++;
        }
        else
            printf("\n");
    }
}

char *fgetline(FILE *fpntr)
{
    static char buff[MAX_LINE_LENGTH + 2];

    int next;
    int pos = 0;
    while ((next = fgetc(fpntr)) != '\n' && next != EOF)
    {
        buff[pos++] = next;
    }
    if (ferror(fpntr))
    {
        perror("Error occured when reading with fgetc()\n");
        exit(EXIT_FAILURE);
    }

    buff[pos] = '\0'; // makes buff a valid C string of correct length

    if (next == EOF && pos == 0) // handles non-newline terminated files
    {
        return NULL;
    }
    return buff;
}