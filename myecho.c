#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

char*
mostrar_variable(char *var){
  char *value;
  value = getenv(var);
  return value;
}

void
myecho(char *argv[],int args){
  char *env;
  char *user;
  int id = 0;
  int i = 0;
  for (i = 1;i<args;i++){
    if(strcmp( argv[i], "CASA") == 0){
      env = mostrar_variable("HOME");
      printf("%s ",env);
    }else if(strcmp( argv[i], "DIRECTORIO") == 0){
      env = mostrar_variable("PWD");
      printf("%s ",env);
    }else if(strcmp( argv[i], "USUARIO") == 0){
      user = getlogin();
      printf("%s ",user);
    }else if (strcmp( argv[i], "*") == 0){
      id = getpid();
      printf("%d ",id);
    }else if (strcmp( argv[i], "-n") == 0){
      continue;
    }else{
      printf("%s ",argv[i]);
    }
  }
}

int
main(int argc, char *argv[])
{
  if (strcmp( argv[1], "-n") == 0){
    myecho(argv,argc);
  }else{
    myecho(argv,argc);
    printf("%s","\n");
  }
	exit(EXIT_SUCCESS);
}