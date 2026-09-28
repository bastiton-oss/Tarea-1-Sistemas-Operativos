#include "headers.h"

int redirecciones(char **args){
    int i= 0;
    int j= 0;

    while(args[i] != NULL){
        if(strcmp(args[i], ">") == 0){
            char *archivo= args[i + 1];
            if(archivo == NULL || strcmp(archivo, "<") == 0 || strcmp(archivo, ">") == 0 || strcmp(archivo, ">>") == 0) return -1;

            int fd= open(archivo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if(fd < 0){
                perror("Error al abrir archivo de salida");
                return -1;
            }

            dup2(fd, 1);
            close(fd);
            i += 2;
        }

        else if(strcmp(args[i], ">>") == 0){
            char *archivo= args[i + 1];
            if(archivo == NULL || strcmp(archivo, "<") == 0 || strcmp(archivo, ">") == 0 || strcmp(archivo, ">>") == 0) return -1;

            int fd= open(archivo, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if(fd < 0){
                perror("Error al abrir archivo de salida");
                return -1;
            }

            dup2(fd, 1);
            close(fd);
            i += 2;
        }

        else if(strcmp(args[i], "<") == 0){
            char *archivo= args[i + 1];
            if(archivo == NULL || strcmp(archivo, "<") == 0 || strcmp(archivo, ">") == 0 || strcmp(archivo, ">>") == 0) return -1;

            int fd= open(archivo, O_RDONLY);
            if(fd < 0){
                perror("Error al abrir archivo de entrada");
                return -1;
            }

            dup2(fd, 0);
            close(fd);
            i += 2;
        }
        else{
            args[j++]= args[i++];
        }
    }

    args[j]= NULL;
    return 0;
}