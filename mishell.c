#include "headers.h"

void setup_shell_signals() {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error configurando SIGINT");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        perror("Error configurando SIGQUIT");
        exit(EXIT_FAILURE);
    }
}

int separar_comandos(char **arg, char **comandos[]){
    int cmd_idx= 0;
    comandos[cmd_idx++]= &arg[0];

    for(int i= 0; arg[i] != NULL; i++){
        if(strcmp(arg[i], "|") == 0){
            if(i == 0 || arg[i-1] == NULL || arg[i+1] == NULL || cmd_idx == MAX_PROCESOS){
                fprintf(stderr, "Tuberia invalida o demasiado larga\n");
                return -1;
            }
            arg[i]= NULL;
            comandos[cmd_idx++]= &arg[i+1];
        }
    }
    comandos[cmd_idx++]= NULL;
    return 0;
}




int main(){
    char cwd[1024];
    char *line=NULL;
    size_t capacidad=0;
    char *arg[1024];


    setup_shell_signals();
    setup_sigchld();

    while (true){
        avisar_jobs();
        if(getcwd(cwd,sizeof(cwd))==NULL){
            perror("getcwd");
            return 1;
        }
        printf("miShell:%s$ ",cwd);
        fflush(stdout);
        ssize_t escrito=getline(&line,&capacidad,stdin);
        if(escrito==-1){
            break;
        }
        else{
            while(escrito > 0 && isspace((unsigned char)line[escrito-1])){
                line[--escrito]= '\0';
            }
            bool background= escrito > 0 && line[escrito-1] == '&';
            char *comando= NULL;
            if(background){
                line[--escrito]= '\0';
                while(escrito > 0 && isspace((unsigned char)line[escrito-1])){
                    line[--escrito]= '\0';
                }
                comando= strdup(line);
                if(comando == NULL){
                    perror("strdup");
                    break;
                }
            }
            char *token =strtok(line," \t\n");
            int i=0;
            bool isPipe= false;


            while(token!=NULL && i < 1023){
                if(strcmp(token, "|") == 0)
                {
                    isPipe= true;
                }
                arg[i]=token;
                i+=1;
                token= strtok(NULL," \t\n");
            }
            arg[i]=NULL;
            if(token != NULL){
                fprintf(stderr, "Demasiados argumentos\n");
                free(comando);
                continue;
            }
            if(i==0){
                free(comando);
                continue;
            }
            if(!isPipe){
                // i incluye el nombre; los internos reciben solo la cantidad de argumentos.
                int resultado= comandos_internos(arg, i-1);
                if(resultado != -1){
                    free(comando);
                    continue;
                }
            }

            char **comandos[MAX_PROCESOS+1];
            if(isPipe && separar_comandos(arg, comandos) < 0){
                free(comando);
                continue;
            }

            sigset_t anterior;
            bloquear_sigchld(&anterior);
            int job= background ? crear_job(comando) : -1;
            free(comando);
            if(background && job < 0){
                sigprocmask(SIG_SETMASK, &anterior, NULL);
                continue;
            }

            if(isPipe){
                pipes(comandos, job, &anterior);
            }
            else{
                pid_t pid= fork();
                if(pid==-1){
                    perror("fork");
                    free(line);
                    return 1;
                }
                else if(pid==0){
                    if(background && setpgid(0, 0) == -1){
                        perror("setpgid");
                        _exit(1);
                    }
                    sigprocmask(SIG_SETMASK, &anterior, NULL);
                    struct sigaction sa_default;
                    sa_default.sa_handler = SIG_DFL;
                    sigemptyset(&sa_default.sa_mask);
                    sa_default.sa_flags = 0;
                    sigaction(SIGINT, &sa_default, NULL);
                    sigaction(SIGQUIT, &sa_default, NULL);

                    if(redirecciones(arg) < 0){
                        exit(1);
                    }

                    if(arg[0] == NULL){
                        exit(0);
                    }

                    execvp(arg[0],arg);
                    perror("execvp");
                    _exit(127);
                }
                else{
                    if(background){
                        agregar_proceso(job, pid);
                        anunciar_job(job);
                    }
                    sigprocmask(SIG_SETMASK, &anterior, NULL);
                    if(!background){
                        while(waitpid(pid, NULL, 0) == -1){
                            if(errno == EINTR) continue;
                            if(errno != ECHILD) perror("waitpid");
                            break;
                        }
                    }
                }
            }
        }
    }
    free(line);
    return 0;
}
