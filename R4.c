#include "headers.h"

void pipes(char ***comandos, int job, const sigset_t *mascara){

	//Contador de comandos
	int i= 0;
	pid_t pid;
	int status;
	pid_t hijos[MAX_PROCESOS];

	//Guarda fd de salida del pipe anterior
	int salidaAnterior= 0;

	//fd pipe creado
	int fd[2];

	while (comandos[i] != NULL){

		if(comandos[i+1] != NULL){
			if(pipe(fd) == -1){
				perror("pipe");
				exit(1);
			}
		}

		pid= fork();

		if (pid == 0){
			if(job >= 0 && setpgid(0, tabla_jobs[job].pid) == -1){
				perror("setpgid");
				_exit(1);
			}
			sigprocmask(SIG_SETMASK, mascara, NULL);

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
			hijos[i]= pid;
			if(job >= 0) agregar_proceso(job, pid);
			if(salidaAnterior != 0){
				close(salidaAnterior);
			}

			if(comandos[i+1] != NULL){
				close(fd[1]);
				salidaAnterior= fd[0];
			}
			else{
				salidaAnterior= 0;
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

	if(job >= 0){
		anunciar_job(job);
	}
	sigprocmask(SIG_SETMASK, mascara, NULL);
	if(job >= 0) return;

	for (int j= 0; j < i; j++){
		while(waitpid(hijos[j], &status, 0) == -1){
			if(errno == EINTR) continue;
			// SIGCHLD puede haber recogido este hijo.
			if(errno != ECHILD) perror("waitpid");
			break;
		}
	}
}
