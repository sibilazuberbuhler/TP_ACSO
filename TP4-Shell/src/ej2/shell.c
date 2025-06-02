#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200

int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count = 0;

    while (1) 
    {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0)
            break;

        if (strstr(command, "||") != NULL) {
            printf("Error: doble pipe\n");
            continue;
        }
        if (command[0] == '|') {
            printf("Error: pipe al inicio\n");
            continue;
        }
        if (strstr(command, "| |") != NULL) {
            printf("Error: pipe vacío entre comandos\n");
            continue;
        }
        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        /* You should start programming from here... */
        for (int i = 0; i < command_count; i++) 
        {
            printf("Command %d: %s\n", i, commands[i]);
        }  
        if (command_count == 0) {
            printf("No ingreso nigun comando\n");
            continue;
        }

        if (command_count > 0) {
            int fd[2], prev_fd = -1;
            pid_t pids[MAX_COMMANDS];

            for (int i = 0; i < command_count; i++) {
                if (i < command_count - 1 && pipe(fd) == -1) {
                    printf("Error al crear pipe\n");
                    exit(1);
                }

                pid_t pid = fork();
                if (pid == 0) {
                    if (i > 0) {
                        dup2(prev_fd, STDIN_FILENO);
                        close(prev_fd);
                    }

                    if (i < command_count - 1) {
                        close(fd[0]);
                        dup2(fd[1], STDOUT_FILENO);
                        close(fd[1]);
                    }

                    char *args[64];
                    int j = 0;
                    char *arg = strtok(commands[i], " ");
                    while (arg != NULL) {
                        if (j >= 63) {
                            printf("Demasiados argumentos para el comando\n");
                            exit(1);
                        }
                        args[j++] = arg;
                        arg = strtok(NULL, " ");
                    }
                    args[j] = NULL;

                    execvp(args[0], args);
                    printf("No se encontro el comando\n");
                    exit(1);
                } 
                else if (pid < 0) {
                    printf("Error al hacer fork\n");
                    exit(1);
                }

                pids[i] = pid;
                if (prev_fd != -1)
                    close(prev_fd);
                if (i < command_count - 1) {
                    close(fd[1]);
                    prev_fd = fd[0];
                }
            }

            for (int i = 0; i < command_count; i++) {
                int status;
                waitpid(pids[i], &status, 0);
            }

            command_count = 0;
        }

    }
    return 0;
}

