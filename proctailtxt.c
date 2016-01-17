#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>


enum{
  MAX=20
};

char*
dar_variable(char *var)
{
  char *value;
  value = getenv(var);
  return value;
}

static void
creatfile(char *name,int bytes){
  int fd;
  int fdaux;
  int nr;
  int nw;
  int buffer[1024];
  char fout[128];

  sprintf(fout,"%s.out",name);

  fd = open (name,O_RDONLY);
  if(fd<0)
    err(1,"Error: open\n");

  fdaux = creat(fout,0644);
  if(fdaux<0)
    err(1,"Error: creat\n");

  if(bytes != 0)
    lseek(fd,bytes*(-1),SEEK_END);

  for(;;){
    nr=read(fd,buffer,1024);
    if(nr<=0)
      break;

    nw = write(fdaux,buffer,nr);
    if(nw!=nr)
      err(1,"Error: erro al escribir\n");
  }
  close(fd);
  close(fdaux);
}

int
estxt(char **p){
  return (*p!=NULL)&&(strlen(*p)==strlen(".txt"));
}

void
waitchild(int txtfiles){
  int i;
  int status;
  int finalstatus=0;

  for(i=0;i<txtfiles;i++){
    wait(&status);
    if (status!=0)
      finalstatus = 1;
  }

  if (finalstatus)
    exit(EXIT_FAILURE);
}

void
forkfiles(char ficheros[][128],int txtfiles,int bytes){
  int i;
  int pid;
  struct stat fichero;

  for(i=0;i<txtfiles;i++){
    pid = fork();
    switch(pid){
    case -1:
        err(1,"Error: fork \n");
        break;
    case 0:
      stat(ficheros[i],&fichero);
      if ((fichero.st_size)<bytes)
        bytes = fichero.st_size;
      creatfile(ficheros[i],bytes);
      exit(EXIT_SUCCESS);
      break;
    }
  }
  waitchild(txtfiles);
}



static void
leerdirp(DIR *dirp,int bytes){
  struct dirent *direntp;
  struct stat info;
  char ficheros[MAX][128];
  char *p;
  int txtfiles = 0;

  while ((direntp = readdir(dirp)) != NULL) {
    char *name;
    name = direntp->d_name;
    stat(name,&info);
    if((info.st_mode & S_IFMT)!=S_IFREG)
      continue;

    p = strstr(name,".txt");
    if(!estxt(&p))
      continue;
    
    strcpy(ficheros[txtfiles],name);
    txtfiles++;
    if(txtfiles==MAX){
      fprintf(stderr,"Error: demasiados ficheros\n");
      exit(1);
    }
  }
  closedir(dirp);
  forkfiles(ficheros,txtfiles,bytes);
}


int
main(int argc, char *argv[])
{
  char *env;
  DIR *dirp;
  char *bytes;
  int intbytes =0;

  env = dar_variable("PWD");
  dirp = opendir(env);

  if (dirp == NULL)
    err(1,"Error: No se puede abrir el directorio\n");

  switch(argc){
  case 1:
    leerdirp(dirp,intbytes);
    break;
  case 2:
    bytes = argv[1];
    intbytes = atoi(bytes);
    if (intbytes == 0){
      printf("Error: argumento N Bytes inválido\n");
      exit(1);
    }
    leerdirp(dirp,intbytes);
    break;
  default:
    printf("Error: número de argumentos inválido\n");
    exit(1);
    break;
  }
  exit(EXIT_SUCCESS);
}
