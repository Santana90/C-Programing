#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>

pthread_mutex_t mutex;
unsigned char  *mem;

void*
runthread(void *file){

  char *fd=(char*)file;
  FILE *fp;
  unsigned char x;
  unsigned char y;
  unsigned char buffer[1024];
  int i;
  int nr;

  fp=fopen (fd,"r");
  if (fp == NULL)
    pthread_exit(NULL);
  while (!feof(fp)) {
    nr=fread(buffer,sizeof(unsigned char),1024,fp);    
    for(i=0;i<nr-1;i++){
      x = buffer [i];
      y = buffer [i+1];
      pthread_mutex_lock(&mutex);
      if (mem[x*128+y]<255)
          mem[x*128+y] = mem[x*128+y] +1;
      pthread_mutex_unlock(&mutex);
      }
  }
  fclose (fp);
  pthread_exit(NULL);
}


void
mostrar(void){
  int i;
  int j;
  for ( i=0;i<128;i++){
    for ( j=0;j<128;j++){
      printf("(%d,%d)=%d\n",i,j,mem[i*128+j]);
    }
  }
} 

void 
readpgm(char *pixmap){
  int fd;
  struct stat s;

  fd=open(pixmap,O_RDONLY);
  if (fd<0)
    err(1,"%s",pixmap);
  stat(pixmap,&s);
  mem = mmap (0, s.st_size, PROT_READ,MAP_SHARED, fd, 0);
  mostrar(); 
  close(fd);
  munmap(mem,s.st_size);
}

void 
createpgm(char *pixmap,int argc, char *argv[]){
  int fd;
  struct stat s;
  pthread_t *th;
  int i;

  fd = open (pixmap, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (fd<0)
    err(1,"%s",pixmap);

  lseek (fd, (128*128)-1, SEEK_SET);
  write(fd,"",1);
  stat(pixmap,&s);
  mem = mmap (0, s.st_size, PROT_READ | PROT_WRITE,MAP_SHARED, fd, 0);
  
  memset(mem,0,s.st_size);
  
  th=malloc(sizeof(pthread_t)*(argc-2));
  pthread_mutex_init(&mutex,NULL);

  for(i=2;i<argc;i++){
    pthread_create(&th[i],NULL,runthread,(void*)argv[i]); // al crear segundo argumento la funcion 
  }
  for(i=2;i<argc;i++)
    pthread_join(th[i],NULL);
    
  close(fd);
  munmap(mem,s.st_size);
}

int
main(int argc, char *argv[] ){
  char *pixmap;

  if (argc<3){
    fprintf(stderr,"Numero de argumentos incorrecto\n");
    exit(1);
  }

  if (strcmp(argv[1],"-p")==0){
    pixmap = argv[2];
    readpgm(pixmap);
    exit (EXIT_SUCCESS);
  }
  pixmap = argv[1];
  createpgm(pixmap,argc,argv);

  exit (EXIT_SUCCESS);
}
