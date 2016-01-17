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

enum{
  N=128
};

int
mytokenize(char *str, char **args,char *delim)
{
  char *token;
  int ntokens = 0;
  token = strtok(str,delim);

  while(token){
    args[ntokens] = token;
    ntokens++;
    token = strtok(NULL,delim);
  }
  return ntokens;
}

static void
makepath(char *path,char *comando){

  char *auxpath;
  int tokens;
  int i;
  char *aux[N];

  if (access(comando,X_OK)==0){
    strcpy(path,comando);
  }else{
    auxpath=getenv("PATH");
    tokens = mytokenize(auxpath,aux,":\n");
    for(i=0;i<tokens;i++){
      sprintf(path,"%s/%s",aux[i],comando);
      if (access(path,X_OK)==0)
        break;
    }
  }
}

static void
mkfifocmd(){
  int fd;

  fd = open("fifocmd.out",O_WRONLY|O_CREAT|O_APPEND,0644);
  if (fd<0)
    err(1,"fifocmd.out");
  dup2(fd,1);
  close(fd);

  fd = creat("/dev/null",0644);
  if (fd<0)
    err(1,"dev/null");
  dup2(fd,2);
  close(fd);
}

static void
closepipe(int p[2]){
  close(p[0]);
  close(p[1]);
}

static void
exec(char *cmd[] , char *comando[]){
  char path[N];
  int pid;
  int pid2;
  int p[2];

  pipe (p);
  pid =fork();
  switch(pid){
  case -1:
    err(1,"Error: fork \n");
    break;
  case 0:
    dup2(p[1],1);//comando al pipe
    closepipe(p);
    makepath(path,cmd[0]);
    execv(path,cmd);
    err(1,"exec:%s",cmd[0]);
  }
  pid2 = fork();
  switch(pid2){
  case -1:
    err(1,"Error: fork \n");
    break;
  case 0:
    dup2(p[0],0); // salida del pipe
    closepipe(p);
    mkfifocmd();
    makepath(path,comando[0]);
    execv(path,comando);
    err(1,"exec:%s",comando[0]);

  }
  closepipe(p);
  wait(NULL);
  wait(NULL);
}

static void 
run(char *path,char *comando[64]){

  FILE *fp;
  char *line;
  char buffer [N];
  char *aux[N];
  int n;
  for(;;){
    fp=fopen (path,"r");
    if(fp==NULL)
      break;
    for(;;){
      line = fgets(buffer ,128,fp);
      if (line == NULL)
        break;
      n = mytokenize(line,aux," \n");
      aux[n]=NULL;
      exec(aux,comando);
    }
    fclose(fp);
  }
}

static void
takecomands(char *comando[64],char *argv[]){
  int j = 2;
  int i = 0;

  while(argv[j]!=NULL){
    comando[i] = argv[j];
    i++;
    j++;
  }
  comando[i]=NULL;
}

static void
manage_fifo(char *path){
   if (access ("fifocmd.out",F_OK)==0)
    truncate("fifocmd.out",0);
   mkfifo (path,0644);
}

int
main(int argc, char *argv[])
{
  char *comando[64];
  char *path;

  if (argc<3){
    fprintf(stderr,"Numero de argumentos incorrecto\n");
    exit(1);
  }
  path = argv[1];

  manage_fifo(path);
  takecomands(comando,argv);
  run(path,comando);

  exit(EXIT_SUCCESS);
}
