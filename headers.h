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


void pipes(char ***comandos);
int redirecciones(char **comandos);
int comandos_internos(char **arg, int i);
int pmon(int seg);
void setup_shell_signals();

#endif
