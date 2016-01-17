#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <err.h>
#include <string.h>


enum{
	N = 128,
	L =256,
};
//Variables vectores para las letras, primeras, ultimas, y el total de ellas.
int letras[L];
int first[L];
int last[L];
int total;

static void 
count(int nr,int iflag,char buffer[]){

	int primera =1;
	int ultima;
	int c;
	int i = 0;

	for(i=0;i<nr;i++){
		c=buffer[i];
		if isalnum(c){
			if(isalpha(c)){
				if(!iflag)
					c = tolower(c);
				if (primera){
					primera=0;
					first[c]=first[c]+1;
				}
				total++;
				letras[c] = letras[c]+1;
			}
		}else{
			primera=1;
			ultima = buffer[i-1];
			if(!iflag){ 
				ultima = tolower(ultima);
			}
			last[ultima] = last[ultima]+1;
		}
	}	
}

static void 
freqcount(int fd , int iflag){

	char buffer[N];
	int nr;
	for(;;){
		nr = read(fd,buffer,N);
		if (nr ==0)
			break;
		count(nr,iflag,buffer);	
		if (nr<0)
			err(1,"read");
	}
	if(fd!=0){
		close(fd);
	}
}

static void
freqargs(int argc, char *argv[],int fd){

	int iflag =0;
	int args =1;
	int i =0;

	switch(argc){
		case 1:
			freqcount(fd,iflag);
			break;
		case 2:
			if(strcmp( argv[1], "-i")==0){
				iflag = 1;
				freqcount(fd,iflag);
			}else{
				fd = open(argv[1],O_RDONLY);
				freqcount(fd,iflag);
			}
			break;
		case 3:
			if(strcmp( argv[1], "-i")==0){
				iflag = 1;
				if(argv[2]==NULL){
					freqcount(fd,iflag);
				}else{
					fd = open(argv[2],O_RDONLY);
					freqcount(fd,iflag);
				}
			}else{
				for (i=args;i<argc;i++){
					fd = open(argv[i],O_RDONLY);
					freqcount(fd,iflag);
				}
			}
			break;
		default:
			if(strcmp( argv[1], "-i")==0){
				iflag = 1;
				args=2;
			}
			for (i=args;i<argc;i++){
				fd = open(argv[i],O_RDONLY);
				freqcount(fd,iflag);
			}
			break;
	}
}

int
main(int argc, char *argv[])
{
	int fd =0;
	int i =0;
	float percent;

	memset(letras,0,256*sizeof(int));

	freqargs(argc,argv,fd);

	for(i=0;i<L;i++){
		if (letras[i]>0){
			percent = ((float)letras[i]*100)/total;
			printf("%c %.2f%% %d %d \n",i,percent,first[i],last[i]);
		}
	}
	if (fd < 0)
		err(1,"No existe: %s",argv[1]);

	exit(EXIT_SUCCESS);
}
