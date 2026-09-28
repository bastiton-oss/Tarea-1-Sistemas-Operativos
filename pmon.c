#include <signal.h>

typedef struct {
    pid_t pid;
    unsigned long tiempousu_a;
    unsigned long tiemposis_a;
    double cpu;
}DatosCPU;

volatile sig_atomic_t b_alarma=0;
volatile sig_atomic_t salir_pmon=0;

void manejar_sigalrm(int sig){
  (void)sig;
  b_alarma = 1;
}

void manejar_sigint_pmon(int sig){
  (void)sig;
  salir_pmon = 1;
}


int leer_stat(pid_t pid,char *estado, unsigned long *tiempo_usu, unsigned long *tiempo_sis){
  int pid_arc;
  char ruta[1000];
  char nombre[100];
  snprintf(ruta,sizeof(ruta),"/proc/%d/stat",pid);
  FILE*archivo=fopen(ruta,"r");
  if(archivo==NULL){
    perror("fopen");
    return 1;
  }
  if(fscanf(archivo,"%d %99s %c",&pid_arc,nombre,estado)!= 3){
    fclose(archivo);
    return 1;
  }
  if(fscanf(archivo,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %lu %lu",
     tiempo_usu, tiempo_sis) != 2){
    fclose(archivo);
    return 1;
  }
  fclose(archivo);
  return 0;
}

int leer_status(pid_t pid, unsigned long *memoria){
  char ruta[1000];
  char linea[256];
  int enc=0;
  snprintf(ruta, sizeof(ruta), "/proc/%d/status", pid);
  FILE *archivo = fopen(ruta,"r");
  if(archivo==NULL){
    perror("fopen");
    return 1;
  }
  while(fgets(linea, sizeof(linea), archivo) != NULL){
    if(strncmp(linea, "VmRSS:", 6) == 0){
      if(sscanf(linea, "VmRSS: %lu", memoria) != 1){
        fclose(archivo);
        return 1;
      }
      enc=1;
      break;
    }
  }
  fclose(archivo);
  if(enc==0){
    return 1;
  }
  return 0;
}

int pmon(int seg){
  struct sigaction alarma;
  struct sigaction sigint;
  struct sigaction ant;
  alarma.sa_handler=manejar_sigalrm;
  sigemptyset(&alarma.sa_mask);
  alarma.sa_flags=SA_RESTART;
  if(sigaction(SIGALRM,&alarma,NULL)== -1) {
    perror("sigaction");
    return 1;
  }
  sigint.sa_handler=manejar_sigint_pmon;
  sigemptyset(&sigint.sa_mask);
  sigint.sa_flags=SA_RESTART;
  if(sigaction(SIGINT,&sigint,&ant)==-1) {
      perror("sigaction");
      return 1;
  }
  b_alarma=1;
  salir_pmon=0;
  while(salir_pmon==0){
    if(b_alarma!=0){
      b_alarma=0;
      printf("\033[H\033[J");
      //aca va lo de R5  
      alarm(seg);
    }    
    pause();
  }
  alarm(0);
  if(sigaction(SIGINT,&ant,NULL)== -1){
    perror("sigaction");
    return 1;
  }
  return 0;
}
