#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <err.h>

char*
dar_variable(char *var)
{
  char *value;
  value = getenv(var);
  return value;
}

void
exec(char *filename, char *rexp , char *comando[]){
  int p[2];
  int fd;
  int pid;
  int pid2;

  pipe (p);
  pid =fork();
  switch(pid){
  case -1:
    err(1,"Error: fork \n");
    break;
  case 0:
    fd = open(filename,O_RDONLY);
    if(fd<0)
      err(1,"Error: open\n");
    dup2(fd,0);       
    close(fd);

    dup2(p[1],1);//comando al pipe
    close(p[0]);//solo tengo que tener abierto 0 1 2
    close(p[1]);
    execvp(comando[0],comando);
    exit(EXIT_FAILURE);
  }

  pid2 = fork();
  switch(pid2){
  case -1:
    err(1,"Error: fork \n");
    break;
  case 0:
    dup2(p[0],0); // salida del pipe
    close(p[0]);//solo tengo que tener abierto 0 1 2
    close(p[1]);
    execlp("grep","grep",rexp,NULL);
    exit(1);
  }
  close(p[0]);//solo tengo que tener abierto 0 1 2 por lo que tengo que cerrar antes de los wait 
  close(p[1]);
  wait(NULL);
  wait(NULL);
}

static void
leerdirp(DIR *dirp,char *rexp,char **comando){
  struct dirent *direntp;
  struct stat info;

  while ((direntp = readdir(dirp)) != NULL) {
    char *filename;
    filename = direntp->d_name;
    stat(filename,&info);
   // printf("%s\n",filename);
    if((info.st_mode & S_IFMT)!=S_IFREG)
      continue;
    
    exec(filename,rexp,comando);
  }
  closedir(dirp);
}

int
main(int argc, char *argv[])
{
  DIR*dirp;
  int j = 2;
  int i = 0;
  char *env;
  char *comando[64];
  char *rexp;

  if (argc<3){
    fprintf(stderr,"Numero de argumentos incorrecto\n");
    exit(1);
  }

  env = dar_variable("PWD");
  dirp = opendir(env);

  if (dirp == NULL)
    err(1,"Error: No se puede abrir el directorio\n");

  rexp = argv[1];
  while(argv[j]!=NULL){
    comando[i] = argv[j];
    i++;
    j++;
  }
  comando[i]=NULL;
  leerdirp(dirp,rexp,comando);
    //expresion regular + comando
  exit(EXIT_SUCCESS);
}
