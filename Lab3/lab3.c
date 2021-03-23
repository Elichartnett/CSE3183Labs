// compile using gcc -Wall -o runpar lab3.c
// call by ./runpar NUMCORES COMMAND... _files_ FILE...

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define F_SETFL 4
#define F_GETFL 3
#define O_NONBLOCK 0x00000004

int run_command_in_subprocess(char *argv_new[4], int pipe[]);
void printout_terminated_subprocess(char *file_name, int rpipe);

int main(int argc, char *argv[])
{
    int errors = 0;

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

        char *argv_new[4];
        argv_new[0] = command;
        argv_new[3] = NULL;

        char *option = NULL;
        if (strcmp(argv[3], "_files_") != 0) //Command has -option
        {
            option = argv[3];
            first_file++;
        }

        int file_num = 0;
        int num_files = argc - first_file;
        int running = 0;
        int pipes[num_files][3]; //Index 0: process id, index 1: read pipe fd, index 2: write pipe fd
        int pfds[2];

        while (file_num < num_files)
        {
            if (running < num_cores)
            {
                char *file = argv[first_file];

                if (option != NULL)
                {
                    argv_new[1] = option;
                    argv_new[2] = file;
                }
                else
                {
                    argv_new[1] = file;
                    argv_new[2] = NULL;
                }

                pipe(pfds); // need to error check
                fcntl(pfds[0], F_SETFL, fcntl(pfds[0], F_GETFL) | O_NONBLOCK);
                fcntl(pfds[1], F_SETFL, fcntl(pfds[1], F_GETFL) | O_NONBLOCK);

                pipes[file_num][1] = pfds[0];
                pipes[file_num][2] = pfds[1];

                pipes[file_num][0] = run_command_in_subprocess(argv_new, pipes[file_num]);

                running++;
                first_file++;
                file_num++;
            }
            else //First clean up function runs when cores are full to open up more space (EX: num_cores = 1 and num_files = 3)
            {
                int status = 0;
                int pid = wait(&status);
                running--;
                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
                {
                    for (int i = 0; i < num_files; i++)
                    {
                        if (pid == pipes[i][0])
                        {
                            printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][1]);
                            break;
                        }
                    }
                }
                else
                    errors = 1;
            }
        }

        while (running > 0) //Second clean up function runs if cores do not over flow (EX: num_cores = 3 and num_files = 3)
        {
            int status = 0;
            int pid = wait(&status);
            running--;
            if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
            {
                for (int i = 0; i < num_files; i++)
                {
                    if (pid == pipes[i][0])
                    {
                        printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][1]);
                        break;
                    }
                }
            }
            else
            {
                printf("Error with exit status\n");
                errors = 1;
            }
        }
    }
    if (errors == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

int run_command_in_subprocess(char *argv_new[4], int pipe[])
{
    int pid = fork();
    switch (pid)
    {
    case -1:
        perror("Fork failed");
        exit(EXIT_FAILURE);

    case 0:               //Child process
        close(pipe[1]);   //Closing read end (this pipe has 3 indexes: pid, read fd, write fd)
        dup2(pipe[2], 1); //Fd 1 is standard output - redirects to pipe write end (pfds[1])
        execvp(argv_new[0], argv_new);
        exit(EXIT_FAILURE);

    default:            //Parent process
        close(pipe[2]); //closing write end (this pipe has 3 indexes: pid, read fd, write fd)
        return pid;
    }
}

void printout_terminated_subprocess(char *file_name, int rpipe)
{
    printf("--------------------\n");
    printf("%s%s\n", "Output from: ", file_name);
    printf("--------------------\n");

    char buff[4096];
    int nread;
    while ((nread = read(rpipe, buff, 4096)) > 0)
        write(1, buff, nread);
    if (nread == -1)
    {
        //FIX - see if there was an error while reading or just nothing to read
    }
}
