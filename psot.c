#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>


enum{
	MAXCMD = 128,
	N = 128
};

typedef struct Tcomando Tcomando;
struct Tcomando
{
	char *comando;
	char *argumentos[64];
};

//Implementación si tengo timeout
static void
offtime(int sig){
	exit(EXIT_FAILURE);
}

int
estst(char **p){
  return (*p!=NULL)&&(strlen(*p)==strlen(".tst"));
}

int
esok(char **p){
  return (*p!=NULL)&&(strlen(*p)==strlen(".ok"));
}

int
esout(char **p){
  return (*p!=NULL)&&(strlen(*p)==strlen(".out"));
}

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
cerrar_pipes(int ncomandos,int p[][2]){
	int i;
	for(i=0;i<ncomandos-1;i++){
		close(p[i][0]);
		close(p[i][1]);
	}
}

static void
entryCmd(){
	int fd;

	fd = open("/dev/null",O_RDONLY);
	if (fd < 0 )
		err(1,"/dev/null");
	dup2(fd,0);
	close(fd);
}

static void
redir_out(char dir[1024],char *fout){
	int fd;

	fd = creat(fout,0644);
	if (fd < 0)
		err(1,"creat\n");
	dup2(fd,1);
	dup2(fd,2);
	close(fd);
}

int
samefiles(char *fok, char *fout){

    FILE *f1;
    FILE *f2;
	char c[N];
    char c2[N];

 	f1 = fopen(fok,"r");
    f2 = fopen(fout,"r");

	if (f1== NULL || f2 == NULL)
		return 0;

    while(!feof(f1) || !feof(f2)){
        fgets(c,N,f1);
        fgets(c2,N,f2);
        if ( strcmp(c,c2) != 0){
            printf ("%s y %s no coinciden \n",fok,fout); // me imprimo los archivos que no coinciden
            return 0;
        }
    }
    fclose(f1);
    fclose(f2);
    return 1;
}

static void
copyfile(char *fout, char *fok){
  int fd;
  int fdaux;
  int nr;
  int nw;
  int buffer[1024];

  fd = open(fout,O_RDONLY);
  if(fd<0)
    err(1,"Error: open\n");

  fdaux = creat(fok,0644);
  if(fdaux<0)
    err(1,"Error: creat\n");

  for(;;){
    nr=read(fd,buffer,1024);
    if(nr<=0)
      break;

    nw = write(fdaux,buffer,nr);
    if(nw!=nr)
      err(1,"Error: error al escribir\n");
  }
  close(fd);
  close(fdaux);
}


static void
exec_pipeline(int ncomandos,Tcomando comandos[MAXCMD],char *file,char dir[1024]){
	char path[N];
	int pid;
	int i = 0;
	int p[N][2];
	char *aux[2];
	char *fileName;
	int estado = 1;
	char fout[N];
	char fok[N];

	mytokenize(file , aux, "." );
	fileName = aux[0];

	sprintf(fout,"%s/%s.out",dir,fileName);

	for(i=0;i<ncomandos-1;i++){
		pipe(p[i]);
	}
	for (i=0;i<ncomandos;i++){
		pid = fork();
		switch(pid){
		case -1:
			err(1,"fork");
			break;
		case 0:
			if (i == 0){ //entrada del pipeline
				entryCmd();
			}
			redir_out(dir,fout); //salida
			if(ncomandos>1){
				if(i==0)
					dup2(p[0][1],1);
				else if (i == ncomandos-1)
					dup2(p[i-1][0],0);
				else{
					dup2(p[i][1],1);
					dup2(p[i-1][0],0);
				}
				cerrar_pipes(ncomandos,p);
			}
			makepath(path,comandos[i].comando);
			execv(path,comandos[i].argumentos);
			err(1,"%s",comandos[i].comando);
			break;
		}
	}
	cerrar_pipes(ncomandos,p);
	for (i=0;i<ncomandos;i++)
		wait(&estado);

	if (estado == 0){ //miro si los estados son OK y sino no compruebo en caso de que el último sea el que falla
		sprintf(fok,"%s/%s.ok",dir,fileName);
		if (access(fok,F_OK) == 0){
			if (!samefiles(fok,fout)){
				estado = 1;
			}else{
				estado = 0;
			}
		}else{
			copyfile(fout,fok);
		}
	}
	if(estado){
		printf("Test fallido %d\n",estado);
		exit(estado);
	}
	printf("Test correcto: %s.tst \n",fileName);
	exit(estado);
}

int
escd(char *aux[N],int n){
	int e;
	//modificacion
	char mod[N];
	char *home;

	if (strcmp(aux[0],"cd") ==0){
		if(n==1){
			home = getenv("HOME");
			e= chdir(home);
			if(e<0)
				err(2,"cd");
		}
		else{
			e = chdir(aux[1]);
			if(e<0){
				sprintf(mod,"%s.dir",aux[1]);
				e = chdir(mod);
				if(e<0)
					err(1,"direccion invalida%s",mod);
			}
		}
		return 1;
	}
	return 0;
}

static void
findenv(char *aux[N], int n){
	int i;
	for (i=0;i<n;i++){
		if (aux[i][0]=='$'){
			aux[i] = getenv(&aux[i][1]);
		}
	}
}

static void
ejecutartest(char *file,int timeout){
	FILE *fp;
	char *linea;
	int ncomandos=0;
	int n;
	char buffer[N];
	char *aux[N];
	Tcomando comandos[MAXCMD];
	int i =0;
	char dir[1024];

	if (timeout){
		//printf("tengo alarma\n");
		signal(SIGALRM,offtime);
		alarm(timeout);
	}

	getcwd (dir,1024);

	fp = fopen(file,"r");
	if (fp == NULL)
		err(1,"fopen\n");

	for (;;){
		linea = fgets(buffer,N,fp);
		if (linea==NULL)
			break;
		n = mytokenize(linea,aux," \n"); // en aux estan mis comandos tokenizados aux[0] ....aux [N]
		if(n==0)
			continue;
		findenv(aux,n);
		if(escd(aux,n))
			continue;
		for (i=0;i<n;i++){
			comandos[ncomandos].argumentos[i] = strdup(aux[i]); // copio la zona de memoria

		}
		comandos[ncomandos].comando = comandos[ncomandos].argumentos[0];
		comandos[ncomandos].argumentos[n] = NULL;
		ncomandos++;

	}
	fclose(fp);
	//printf("comandos :%d \n",ncomandos);
	exec_pipeline(ncomandos,comandos,file,dir);
}

static void
deletefiles(DIR *dirp){

	struct dirent *direntp;
	char *pok;
	char *pout;

	while ((direntp = readdir(dirp)) != NULL) {
	    char *name;
	    name = direntp->d_name;
	    pok = strstr(name,".ok");
	    pout = strstr(name,".out");
		if (esok(&pok)||esout(&pout)){
			unlink(name);
		}
	}

}

int
main(int argc, char *argv[])
{
	DIR *dirp;
	struct dirent *direntp;
	char *p;
	char *env;
	int pid;
	int i = 0;
	int nfiles =0;
	//opcional1
	int timeout =0 ;
	//opcional2
	int eraser = 0;

	if (argc == 3){
		if (strcmp(argv[1],"-t")==0){
			timeout = atoi(argv[2]);
		}
	}
	if (argc == 2){
		if (strcmp(argv[1],"-c")==0){
			eraser = 1;
		}
	}

	env = getenv("PWD");
	dirp = opendir(env);

	if (dirp == NULL)
		err(1,"Error: No se puede abrir el directorio\n");

	if(eraser){
		deletefiles(dirp);
		exit(EXIT_SUCCESS);
	}

	while ((direntp = readdir(dirp)) != NULL) {
	    char *name;
	    name = direntp->d_name;
	    p = strstr(name,".tst");
		if (estst(&p)){
			nfiles++;
			pid = fork();
			switch(pid){
			case -1:
				err(1,"fork \n");
				break;
			case 0:
				ejecutartest(name,timeout);
				exit(EXIT_SUCCESS);
			}
		}
	}
	for (i=0 ;i<nfiles;i++){
		wait(NULL);
	}
	closedir(dirp);
	exit(EXIT_SUCCESS);
}
