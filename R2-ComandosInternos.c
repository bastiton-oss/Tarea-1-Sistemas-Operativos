#include "headers.h"

int comandos_internos(char **arg,int i){
  char *ruta=NULL;
  if(strcmp(arg[0],"cd")==0){
    if(arg[1]==NULL){
      if(getenv("HOME")==NULL){
         fprintf(stderr,"HOME no esta definido");
         return 1;
      }
      else{  
        ruta= getenv("HOME");
      }
    }
    else{
      ruta=arg[1];
    }
    if(chdir(ruta)==0){
      return 0;
    }
    else{
      perror("chdir");
      return 1;
    }
  }
  else if(strcmp(arg[0],"exit")==0){
    if(arg[1]==NULL){
      exit(0);
    }
    else if(i>=2){
      return 1;
    }
    else{
      int j=0; 
      while(true){
        if(arg[1][j]=='\0'){
          break;
        }
        else{
          if(isdigit(arg[1][j])!=0){
            j+=1;
          }
          else{
            return 1;
          }
        }
      }
      int dig=atoi(arg[1]);
      exit(dig);
    }
  }
  else if(strcmp(arg[0],"jobs")==0){
    return jobs(arg,i);
  }
  else if(strcmp(arg[0],"pmon")==0){
    int seg=2;
    if(i==0){
      return pmon(2);
    }
    else if(i==1){
      int j=0;
      while(true){
        if(arg[1][j]=='\0'){
          break;
        }
        else{
          if(isdigit(arg[1][j])!=0){
            j+=1;
          }
          else{
            return 1;
          }
        }
      }
      seg=atoi(arg[1]);
      if(seg<1){
        return 1;
      }
      return pmon(seg);
    }
    else{
      return 1;
    }
  }
  else{
    return -1;
  }
}
