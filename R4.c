#include "headers.h"

void pipes(char ***comandos){

	//Contador de comandos
	int i= 0;
	pid_t pid;
	int status;

	//Guarda fd de salida del pipe anterior
	int salidaAnterior= 0;

	//fd pipe creado
	int fd[2];

	while (comandos[i] != NULL){

		if(comandos[i+1] != NULL){
			pipe(fd);
		}

		pid= fork();

		if (pid == 0){

			if(salidaAnterior != 0){
				dup2(salidaAnterior, 0);
				close(salidaAnterior);
			}

			if(comandos[i+1] != NULL){
				dup2(fd[1], 1);

				close(fd[1]);
				close(fd[0]);
			}

			struct sigaction sa_default;
            sa_default.sa_handler = SIG_DFL;
            sigemptyset(&sa_default.sa_mask);
            sa_default.sa_flags = 0;
            sigaction(SIGINT, &sa_default, NULL);
            sigaction(SIGQUIT, &sa_default, NULL);

			if(redirecciones(comandos[i]) < 0){
				exit(1);
			}

			if(comandos[i][0] == NULL){
				exit(0);
			}
			
			execvp(comandos[i][0], comandos[i]);
			perror("Error al ejecutar comando");
			exit(1);

		}

		else if(pid > 0){
			if(salidaAnterior != 0){
				close(salidaAnterior);
			}

			if(comandos[i+1] != NULL){
				close(fd[1]);
				salidaAnterior= fd[0];
			}

			i++;
		}

		else if(pid < 0){ //edge case por si falla el fork
			perror("Error al crear fork");

			if (comandos[i+1] != NULL) {
                close(fd[0]);
                close(fd[1]);
            }

            if (salidaAnterior != 0) {
                close(salidaAnterior);
            }

			exit(1);
		}
	}

	if (salidaAnterior != 0) {
    close(salidaAnterior);
	}

	for (int j= 0; j < i; j++){
		waitpid(-1, &status, 0);
	}
}