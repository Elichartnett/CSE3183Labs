// compile using gcc -Wall -o runpar lab3.c
// call by ./runpar NUMCORES COMMAND... _files_ FILE...

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void run_command_in_subprocess(char *file, char *argv_new[4]);
//??? printout_terminated_subprocess(int status, ???);

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
        while (first_file < argc) //makes sure all files run
        {
            if (running < num_cores) //limit number of files running in parallel
            {
                char *argv_new[4];
                argv_new[0] = command;
                argv_new[1] = option;
                argv_new[2] = argv[first_file++];
                argv_new[3] = NULL;

                running++;
                run_command_in_subprocess(argv_new[2], argv_new);
                sleep(3);
            }
            else
            {
                printf("waiting\n");
                wait(NULL);
                running--;
            }
        }

        return EXIT_SUCCESS;
    }
}

void run_command_in_subprocess(char *file, char *argv_new[4])
{
    int pid = fork();
    switch (pid)
    {
    case -1:
        perror("Fork failed");
        exit(EXIT_FAILURE);

    case 0: //child process
        printf("%s%d\n", "Child: ", (int)getpid());
        execvp(argv_new[0], argv_new);
        exit(EXIT_FAILURE);

    default: //parent process
        printf("%s%d\n", "Parent: ", (int)getpid());
        break;
    }
}
