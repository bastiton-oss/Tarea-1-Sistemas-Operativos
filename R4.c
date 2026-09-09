#include "headers.h"



void pipes(char ***comandos){

	//Contador de comandos
	int i= 0;
	pid_t pid;
	int status;

	while (comandos[i] != NULL){
		pid= fork();

		if (pid == 0){

			execvp(comandos[i][0], comandos[i]);
			perror("Error al ejecutar comando");
			exit(1);

		}

		else if(pid > 0){
			i++;
		}

		else if(pid < 0){

			perror("Error al crear fork");
			exit(1);
		}
	}

	for (int j= 0; j < i; j++){
		waitpid(-1, &status, 0);
	}
}