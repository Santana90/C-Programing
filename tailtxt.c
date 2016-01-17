#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


char*
dar_variable(char *var)
{
  char *value;
  value = getenv(var);
  return value;
}

static void
mostrar_fich(char *name,int posicion){
  int fd;
  int nr;
  int nw;
  int buffer[1024];

  fd = open (name,O_RDONLY);
  if(fd<0){
    fprintf(stderr, "Error: %s\n",name);
  }
  if(posicion != 0)
    lseek(fd,posicion*(-1),SEEK_END);
  for(;;){
    nr=read(fd,buffer,1024);
    if(nr<=0)
      break;
    nw = write(1,buffer,nr);
    if(nw!=nr)
      err(1,"Error: erro al leer\n");
  }
  close(fd);
}

static void
leerdirp(DIR *dirp,int bytes){
  struct dirent *direntp;
  struct stat info;
  struct stat fichero;
  char *p;
  while ((direntp = readdir(dirp)) != NULL) {
    char *name;
    name = direntp->d_name;
    stat(name,&info);
    if((info.st_mode & S_IFMT)==S_IFREG){//miro si es fichero
      p = strstr(name,".txt");
      if((p!=NULL)&&(strlen(p)==strlen(".txt"))){ //aqui veo cuando los archivos son solo .txt
        stat(name,&fichero);
        printf("%s\n",name);
        if ((fichero.st_size)<bytes)
          bytes = 0;
        mostrar_fich(name,bytes);
      }
    }
  }
  closedir(dirp);
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
  if (dirp == NULL){
    err(1,"Error: No se puede abrir el directorio\n");
  }

  switch(argc){
    case 1:
      leerdirp(dirp,intbytes);
      break;
    case 2:
      bytes = argv[1];
      intbytes = atoi(bytes);
      if (intbytes != 0){
        leerdirp(dirp,intbytes);
      }else{
        printf("Error: argumento N Bytes inválido\n");
        exit(1);
      }
      break;
    default:
      printf("Error: número de argumentos inválido\n");
      exit(1);
      break;
  }
  exit(EXIT_SUCCESS);
}
