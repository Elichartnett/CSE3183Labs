// Compile using gcc -Wall -o runpar lab3.c
// Call by ./runpar NUMCORES COMMAND... _files_ FILE...
// Create Xmb file with name tX for testing - dd if=/dev/urandom of=tX bs=1m count=X

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/errno.h>

#define F_SETFL 4
#define F_GETFL 3

int run_command_in_subprocess(char *argv_new[4], int pipe[]);
int printout_terminated_subprocess(char *file_name, int rpipe, int errors);

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
        int pipes[num_files][3]; //Index 0: read pipe fd, index 1: write pipe fd, index 2: process id
        int pfds[2];

        while (file_num < num_files) //Makes sure every file is forked
        {
            if (running >= num_cores) //Hit max number of cores in use
            {
                int status = 0;
                int pid = wait(&status);
                running--;
                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
                {
                    for (int i = 0; i < num_files; i++)
                    {
                        if (pid == pipes[i][2])
                        {
                            errors = printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][0], errors);
                            break;
                        }
                    }
                }
                else
                {
                    for (int i = 0; i < num_files; i++)
                    {
                        if (pid == pipes[i][2])
                        {
                            errors = printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][0], errors);
                            break;
                        }
                    }
                    errors = 1;
                }
            }
            else
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

                if (pipe(pfds) == -1) //Error making pipe results in file being skipped, but will attempt another piep on next file
                {
                    errors = 1;
                    first_file++;
                    file_num++;
                }
                else
                {
                    fcntl(pfds[0], F_SETFL, fcntl(pfds[0], F_GETFL) | O_NONBLOCK);
                    fcntl(pfds[1], F_SETFL, fcntl(pfds[1], F_GETFL) | O_NONBLOCK);

                    pipes[file_num][0] = pfds[0];
                    pipes[file_num][1] = pfds[1];

                    pipes[file_num][2] = run_command_in_subprocess(argv_new, pipes[file_num]);

                    running++;
                    first_file++;
                    file_num++;
                }
            }
        }

        while (running > 0) //Some processes may still be running if max number of cores was not reached - if max number of cores was reached, there will be num_cores processes still remaining
        {
            int status = 0;
            int pid = wait(&status);
            running--;
            if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
            {
                for (int i = 0; i < num_files; i++)
                {
                    if (pid == pipes[i][2])
                    {
                        errors = printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][0], errors);
                        break;
                    }
                }
            }
            else
            {
                for (int i = 0; i < num_files; i++)
                {
                    if (pid == pipes[i][2])
                    {
                        errors = printout_terminated_subprocess(argv[argc - num_files + i], pipes[i][0], errors);
                        break;
                    }
                }
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

    case 0:
        close(pipe[0]);   //Closing read end of child process
        dup2(pipe[1], 1); //Fd 1 is standard output - redirects to pipe write end (pfds[1])
        dup2(pipe[1], 2); //Fd 2 is standard error
        execvp(argv_new[0], argv_new);
        exit(EXIT_FAILURE);

    default:
        close(pipe[1]); //Closing right end of parent process
        return pid;
    }
}

int printout_terminated_subprocess(char *file_name, int rpipe, int errors)
{
    printf("--------------------\n");
    printf("%s%s\n", "Output from: ", file_name);
    printf("--------------------\n");

    char buff[4096];
    int nread;
    while ((nread = read(rpipe, buff, 4096)) > 0)
        write(1, buff, nread);
    if (nread < 0)
    {
        errors = 1;
        fflush(stdout);
        if (errno == EAGAIN)
            printf("Child had no output\n");

        else
            printf("Error reading pipe\n");
    }
    return errors;
}
