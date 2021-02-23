// Team: Eli Hartnett (ech364) and Tara Broome (tb2120)
// Compile using gcc -Wall -o lab1 lab1.c
// Valid calls: ./lab1 [style] <filename(s)> or ./lab1 [style] or ./lab1 <filename(s)> or ./lab1

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#define MAX_LINE_LENGTH 100

void process_stream(FILE *fpntr, char bstylechar);
char *fgetline(FILE *fpntr);

int main(int argc, char *argv[])
{
    bool bopt = false;
    char *bstyle = "t"; //a-number all lines; t(default)-number only non-empty lines; n-no line numbering
    char bstylechar = 't';
    int first_file = 1;

    if (argc > 1 && strncmp(argv[1], "-b", 2) == 0 && strlen(argv[1]) == 3) //checking for valid options (-ba, -bt (default), or -bn)
    {
        bstyle = argv[1] + 2; //strips off -b in option
        bstylechar = bstyle[0];

        if (bstylechar == 'a' || bstylechar == 't' || bstylechar == 'n')
        {
            bopt = true;
            first_file = 2;
        }
        else
        {
            printf("%s: invalid body numbering style: '%c'\n", argv[0], bstylechar);
            exit(EXIT_FAILURE);
        }
    }

    if (argc == 1 || (argc == 2 && bopt == true)) //handles command or command + option (both std in)
    {
        process_stream(stdin, bstylechar);
    }

    if (argc >= 2) //handles command + file(s); command + option + file(s)
    {
        for (int i = first_file; i < argc; i++)
        {
            FILE *fpntr;
            if ((fpntr = fopen(argv[i], "r")) == NULL)
                fprintf(stderr, "\nError opening file %s: %s\n", argv[i], strerror(errno));

            else
            {
                process_stream(fpntr, bstylechar);
                fclose(fpntr);
                printf("\n");
            }
        }
    }

    return EXIT_SUCCESS;
}

void process_stream(FILE *fpntr, char bstylechar)
{
    char *line;
    static int count = 1;
    char *spacer = "       ";

    while ((line = fgetline(fpntr)) != NULL)
    {
        switch (bstylechar)
        {
        case 'a':
            printf("%6d  %s", count, line);
            break;

        case 't':
            if (strcmp(line, "\n\0") == 0)
            {
                printf("\n");
                count--;
            }
            else
                printf("%6d  %s", count, line);
            break;

        case 'n':
            printf("%s%s", spacer, line);
            break;
        }
        count++;
    }
}

char *fgetline(FILE *fpntr)
{
    static char buff[MAX_LINE_LENGTH + 2];

    int next = fgetc(fpntr);
    if (ferror(fpntr))
    {
        perror("Error occured when reading with fgetc()");
        fclose(fpntr);
        exit(EXIT_FAILURE);
    }
    if (next == EOF) //if file is empty
        return NULL;
    if (next == '\n') //if only contains new line
    {
        buff[0] = '\n';
        buff[1] = '\0';
        return buff;
    }

    int next_next = fgetc(fpntr);
    if (ferror(fpntr))
    {
        perror("Error occured when reading with fgetc()");
        fclose(fpntr);
        exit(EXIT_FAILURE);
    }

    int pos = 0;

    while (next_next != '\n' && next_next != EOF)
    {
        buff[pos] = next;
        pos++;
        next = next_next;
        next_next = fgetc(fpntr);
    }

    if (next_next == '\n')
    {
        if (pos < MAX_LINE_LENGTH)
        {
            buff[pos] = next;
            buff[pos + 1] = '\n';
            buff[pos + 2] = '\0';
        }
        else
        {
            buff[MAX_LINE_LENGTH] = '\n';
            buff[MAX_LINE_LENGTH + 1] = '\0';
        }
        return buff;
    }

    if (next_next == EOF)
    {
        if (pos < MAX_LINE_LENGTH)
        {
            buff[pos] = next;
            buff[pos + 1] = '\0';
        }
        else
        {
            buff[MAX_LINE_LENGTH + 1] = '\0';
        }
        return buff;
    }

    return buff;
}