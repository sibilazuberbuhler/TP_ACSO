#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 4){ printf("Uso: anillo <n> <c> <s> \n"); exit(0);}

    int n = atoi(argv[1]), c = atoi(argv[2]), s = atoi(argv[3]);

    if (n < 3 || s < 0 || s >= n || c < 0) {
	printf("Argumentos inválidos.\n");
        exit(EXIT_FAILURE);
    }

    int pipes[n][2];
	int pipe_padre[2];
	pipe(pipe_padre);

    for (int i = 0; i < n; i++)
        if (pipe(pipes[i]) < 0) { perror("pipe"); exit(1); }

    for (int i = 0; i < n; i++) {
        if (fork() == 0) {
            int prev = (i + n - 1) % n;
            close(pipes[prev][1]); 
            close(pipes[i][0]);    

            int val;
            read(pipes[prev][0], &val, sizeof(int));
            val++;

            if (i == s) {
				close(pipe_padre[0]); 
				write(pipe_padre[1], &val, sizeof(int));  
				close(pipe_padre[1]);  
            } else {
                write(pipes[i][1], &val, sizeof(int));
            }

            close(pipes[prev][0]);
            close(pipes[i][1]);
            exit(0);
        }
    }

	for (int i = 0; i < n; i++) close(pipes[i][0]);

	write(pipes[s][1], &c, sizeof(int));
	close(pipes[s][1]);

	close(pipe_padre[1]);

	int resultado_final;
	read(pipe_padre[0], &resultado_final, sizeof(int));
	close(pipe_padre[0]);

	printf("El proceso padre recibe el valor final: %d\n", resultado_final);

	for (int i = 0; i < n; i++) wait(NULL);
	return 0;
}

