#include <headers.h>

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
  DatosCPU datos[MAX_JOBS];
  for(int i=0;i<MAX_JOBS;i++){
    datos[i].pid=0;
    datos[i].tiempousu_a=0;
    datos[i].tiemposis_a=0;
    datos[i].cpu=0
  }
  while(salir_pmon==0){
    if(b_alarma!=0){
      b_alarma=0;
      printf("\033[H\033[J");
      printf("PID COMANDO ESTADO %%CPU RSS\n");
      for(int i=0;i<MAX_JOBS;i++){
        if(tabla_jobs[i].numero==0){
          continue;
        }
        if(tabla_jobs[i].pendientes==0){
          continue;
        }
        char estado;
        unsigned long tiempo_usu;
        unsigned long tiempo_sis;
        unsigned long memoria;
        if(leer_stat(pid,&estado,&tiempo_usu,&tiempo_sis)!=0){
          continue;
        }
        if(leer_status(pid,&memoria)!=0){
          memoria=0;
        }
        double cpu=0
        if(datos[i].pid==pid){
          unsigned long tiempo_ant= datos[i].tiempousu_a+datos[i].tiemposis_a;
          unsigned long tiempo_act= datos[i].tiempo_usu+datos[i].tiempo_sis;
          unsigned long resultado=tiempo_act-tiempo_ant;
          cpu=((double)resultado/(double)sysconf(_SC_CLK_TCK))/(double)seg*100.0;
        }
        datos[i].pid=pid;
        datos[i].tiempousu_a=tiempo_usu;
        datos[i].tiemposis_a=tiempo_sis;
        datos[i].cpu=cpu;
        char *estado;
        if(estado=='R'){
          estado="ejecutando";
        }
        else if(estado=='S'){
          estado="durmiendo";
        }
        else if(estado=='Z'){
          estado="zombie";
        }
        else{
          estado="desconocido";
        }
        printf("%d %s %s %.1f %lu\n",pid,tabla_jobs[i].comando,estado,cpu,memoria);
      }
      fflush(stdout);
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
