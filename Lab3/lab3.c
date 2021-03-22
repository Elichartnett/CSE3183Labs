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

void run_command_in_subprocess(char *argv_new[4], int pipe[]);
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
        if (strcmp(argv[3], "_files_") != 0) //command has -option
        {
            option = argv[3];
            first_file++;
        }

        int file_num = 0;
        int num_files = argc - first_file;
        int running = 0;
        int pipes[num_files][2]; //index 0: read pipe fd, index 1: write pipe fd
        int num_printed = 0;

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

                pipe(pipes[file_num]); // need to error check
                fcntl(pipes[file_num][0], F_SETFL, fcntl(pipes[file_num][0], F_GETFL) | O_NONBLOCK);
                fcntl(pipes[file_num][1], F_SETFL, fcntl(pipes[file_num][1], F_GETFL) | O_NONBLOCK);

                run_command_in_subprocess(argv_new, pipes[file_num]);

                running++;
                first_file++;
                file_num++;
            }
            else //first clean up function runs when cores are full (EX: num_cores = 1 and num_files = 3)
            {
                int status = 0;
                int pid = wait(&status);
                printf("Got: %d\n", pid);
                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
                {
                    for (int i = 0; running >= num_cores; i++)
                    {
                        char scurr_pid[6];
                        int nread = read(pipes[i][0], scurr_pid, 5); //switch to findind delimeter not 5 chars
                        {
                            if (nread != -1)
                            {
                                int curr_pid = atoi(scurr_pid);
                                printf("Read: %d\n", curr_pid);
                                if (curr_pid == pid)
                                {
                                    printout_terminated_subprocess(argv[argc - num_files + num_printed], pipes[i][0]); //FIX - need data structure mapping PID to pipe
                                    num_printed++;
                                    running--;
                                }
                                else
                                {
                                    lseek(pipes[i][0], 0, SEEK_SET);
                                }
                            }
                        }
                    }
                }
                else
                    errors = 1;
            }
        }
        if (running > 0) //Second clean up function runs when max num of cores is not reached (EX: num_cores = 3, num_files = 2)
        {
            for (int i = 0; i < running; i++)
            {
                int status = 0;
                int pid = wait(&status);
                printf("Got: %d\n", pid);
                if (WIFEXITED(status) && WEXITSTATUS(status) == EXIT_SUCCESS)
                {
                    for (int i = 0; running > 0; i++)
                    {
                        char scurr_pid[6];
                        int nread = read(pipes[i][0], scurr_pid, 5); //switch to findind delimeter not 5 chars
                        {
                            if (nread != -1)
                            {
                                int curr_pid = atoi(scurr_pid);
                                printf("Read: %d\n", curr_pid);
                                if (curr_pid == pid)
                                {
                                    printout_terminated_subprocess(argv[argc - num_files + num_printed], pipes[i][0]); //FIX - need data structure mapping PID to pipe
                                    num_printed++;
                                    running--;
                                }
                                else
                                {
                                    lseek(pipes[i][0], 0, SEEK_SET);
                                }
                            }
                        }
                    }
                }
                else
                    errors = 1;
            }
        }
    }
    if (errors == 0)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}

void run_command_in_subprocess(char *argv_new[4], int pipe[])
{
    switch (fork())
    {
    case -1:
        perror("Fork failed");
        exit(EXIT_FAILURE);

    case 0: //child process
        close(pipe[0]);
        dup2(pipe[1], 1); //fd 1 is standard output - redirects to pipe write end (pfds[1])
        char pid[6];
        int length = sprintf(pid, "%d", getpid());
        write(pipe[1], pid, length);

        execvp(argv_new[0], argv_new);
        exit(EXIT_FAILURE);

    default: //parent process
        printf("%d\n", getpid());
        close(pipe[1]);
        break;
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
