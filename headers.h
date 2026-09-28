#ifndef HEADERS_H
#define HEADERS_H

#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_JOBS 64
#define MAX_PROCESOS 64

typedef struct {
    int numero;
    pid_t pid;
    pid_t procesos[MAX_PROCESOS];
    int cantidad;
    volatile sig_atomic_t terminados[MAX_PROCESOS];
    volatile sig_atomic_t pendientes;
    char *comando;
} Job;

extern Job tabla_jobs[MAX_JOBS];

void pipes(char ***comandos, int job, const sigset_t *mascara);
int redirecciones(char **comandos);
int comandos_internos(char **arg, int i);
int pmon(int seg);
void setup_shell_signals();
void setup_sigchld();
void bloquear_sigchld(sigset_t *anterior);
int crear_job(const char *comando);
void agregar_proceso(int job, pid_t pid);
void anunciar_job(int job);
void avisar_jobs();
int jobs(char **arg, int i);

#endif
