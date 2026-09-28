#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h> 

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

int main(){
    char cwd[1024];
    char *line=NULL;
    size_t capacidad=0;
    char *arg[1024];


    setup_shell_signals();

    while (true){
        if(getcwd(cwd,sizeof(cwd))==NULL){
            perror("getcwd");
            return 1;
        }
        printf("miShell:%s$ ",cwd);
        ssize_t escrito=getline(&line,&capacidad,stdin);
        if(escrito==-1){
            break;
        }
        else{
            char *token =strtok(line," \n"); 
            int i=0;
            while(token!=NULL){
                arg[i]=token;
                i+=1;
                token= strtok(NULL," \n"); 
            }
            arg[i]=NULL;
            if(i==0){
                continue;
            }
            else if(strcmp(arg[0],"exit")==0){
                break;
            }
            
            pid_t pid= fork();
            if(pid==-1){
                printf("fork");
                break;
            }
            else if(pid==0){
                struct sigaction sa_default;
                sa_default.sa_handler = SIG_DFL;
                sigemptyset(&sa_default.sa_mask);
                sa_default.sa_flags = 0;
                sigaction(SIGINT, &sa_default, NULL);
                sigaction(SIGQUIT, &sa_default, NULL);

                execvp(arg[0],arg);
                perror("execvp");
                _exit(127);
            }
            else{
                waitpid(pid, NULL, 0);
            }
        }
    }
    free(line);
    return 0;
}