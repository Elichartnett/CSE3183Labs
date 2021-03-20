// compile using gcc -Wall -o runpar lab3.c
// call by ./runpar NUMCORES COMMAND... _files_ FILE...

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int run_command_in_subprocess(char *file, char *argv_new[4]);
void printout_terminated_subprocess(int pfds, char *file_name);

int main(int argc, char *argv[])
{
    if ((argc - 1) < 4)
    {
        printf("%s %d %s", "Error: Only", argc - 1, "arguments given. To use runpar, use the following syntax: ./runpar NUMCORES COMMAND... _files_ FILE...\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        int num_cores = atoi(argv[1]);
        char *command = argv[2];

        int first_file = 4;
        char *option = NULL;
        if (strcmp(argv[3], "_files_") != 0) //command has -option
        {
            option = argv[3];
            first_file++;
        }

        int running = 0;
        int num_files = argc - first_file;
        int pfds[num_files];
        int file_num = 0;
        int num_printed = 0;
        while (file_num < num_files)
        {
            if (running < num_cores)
            {
                char *argv_new[4];
                argv_new[0] = command;
                argv_new[1] = option;
                argv_new[2] = argv[first_file++];
                argv_new[3] = NULL;

                running++;
                pfds[file_num++] = run_command_in_subprocess(argv_new[2], argv_new);
            }
            else
            {
                wait(NULL);
                running--;
                printout_terminated_subprocess(pfds[num_printed], argv[argc - num_files + num_printed]);
                num_printed++;
            }
        }

        if (running > 0)
        {
            for (int i = 0; i < running; i++)
            {
                wait(NULL);
                printout_terminated_subprocess(pfds[num_printed], argv[argc - num_files + num_printed]);
                num_printed++;
            }
        }
    }
    return EXIT_SUCCESS;
}

int run_command_in_subprocess(char *file, char *argv_new[4])
{
    int pfds[2];
    if (pipe(pfds) == -1)
    {
        //error creating pipe
    }
    else
    {
        switch (fork())
        {
        case -1:
            perror("Fork failed");
            exit(EXIT_FAILURE);

        case 0: //child process
            //close(pipe(&pfds[0]));
            dup2(pfds[1], 1); //fd 1 is standard output - redirects to pipe write end (pfds[1])
            execvp(argv_new[0], argv_new);
            exit(EXIT_FAILURE);

        default: //parent process
            //close(pipe(&pfds[1]));
            return pfds[0];
            break;
        }
    }
    return pfds[0];
}

void printout_terminated_subprocess(int pfds, char *file_name)
{
    printf("--------------------\n");
    printf("%s%s\n", "Output from: ", file_name);
    printf("--------------------\n");

    char buff[4096];
    int nread = read(pfds, buff, 4096);
    write(1, buff, nread);
}
