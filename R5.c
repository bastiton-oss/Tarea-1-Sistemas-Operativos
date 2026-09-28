#include "headers.h"

Job tabla_jobs[MAX_JOBS];
static int siguiente_job= 1;

static void manejar_sigchld(int sig){
    (void)sig;
    int errno_anterior= errno;
    int status;
    pid_t pid;

    while((pid= waitpid(-1, &status, WNOHANG)) > 0){
        for(int i= 0; i < MAX_JOBS; i++){
            if(tabla_jobs[i].numero == 0) continue;

            for(int j= 0; j < tabla_jobs[i].cantidad; j++){
                if(tabla_jobs[i].procesos[j] == pid && !tabla_jobs[i].terminados[j]){
                    tabla_jobs[i].terminados[j]= 1;
                    tabla_jobs[i].pendientes--;
                    break;
                }
            }
        }
    }
    errno= errno_anterior;
}

void setup_sigchld(){
    struct sigaction sa;
    sa.sa_handler= manejar_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags= SA_RESTART | SA_NOCLDSTOP;

    if(sigaction(SIGCHLD, &sa, NULL) == -1){
        perror("sigaction");
        exit(1);
    }
}

void bloquear_sigchld(sigset_t *anterior){
    sigset_t mascara;
    sigemptyset(&mascara);
    sigaddset(&mascara, SIGCHLD);
    if(sigprocmask(SIG_BLOCK, &mascara, anterior) == -1){
        perror("sigprocmask");
        exit(1);
    }
}

// Se registra con SIGCHLD bloqueada para no perder hijos que terminen rapido.
int crear_job(const char *comando){
    for(int i= 0; i < MAX_JOBS; i++){
        if(tabla_jobs[i].numero == 0){
            char *copia= strdup(comando);
            if(copia == NULL){
                perror("strdup");
                return -1;
            }
            memset(&tabla_jobs[i], 0, sizeof(Job));
            tabla_jobs[i].numero= siguiente_job++;
            tabla_jobs[i].comando= copia;
            return i;
        }
    }
    fprintf(stderr, "No hay espacio para mas jobs\n");
    return -1;
}

void agregar_proceso(int job, pid_t pid){
    Job *actual= &tabla_jobs[job];
    if(actual->cantidad == 0) actual->pid= pid;
    if(setpgid(pid, actual->pid) == -1 && errno != EACCES && errno != ESRCH){
        perror("setpgid");
    }
    actual->procesos[actual->cantidad++]= pid;
    actual->pendientes++;
}

void anunciar_job(int job){
    printf("[%d] %d\n", tabla_jobs[job].numero, tabla_jobs[job].pid);
}

void avisar_jobs(){
    sigset_t anterior;
    bloquear_sigchld(&anterior);
    for(int i= 0; i < MAX_JOBS; i++){
        if(tabla_jobs[i].numero != 0 && tabla_jobs[i].pendientes == 0){
            printf("[%d]+ Done %s\n", tabla_jobs[i].numero, tabla_jobs[i].comando);
            free(tabla_jobs[i].comando);
            memset(&tabla_jobs[i], 0, sizeof(Job));
        }
    }
    sigprocmask(SIG_SETMASK, &anterior, NULL);
}

int jobs(char **arg, int i){
    (void)arg;
    if(i != 0){
        fprintf(stderr, "Uso: jobs\n");
        return 1;
    }

    avisar_jobs();
    sigset_t anterior;
    bloquear_sigchld(&anterior);
    for(int j= 0; j < MAX_JOBS; j++){
        if(tabla_jobs[j].numero != 0){
            printf("[%d] %d Ejecutando %s\n", tabla_jobs[j].numero,
                   tabla_jobs[j].pid, tabla_jobs[j].comando);
        }
    }
    sigprocmask(SIG_SETMASK, &anterior, NULL);
    return 0;
}
