#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static void
eschar(char *str,int *i)
{
	while(isalpha(str[(*i)])){
		(*i)++;
	}
	str[(*i)]='\0';
	(*i)++;
}

static void
nochar(char *str,int *i)
{
	while(isspace(str[(*i)])){
		(*i)++;
	}
}

int
mytokenize(char *str, char **args, int maxargs)
{
	int ntokens=0;
	int i = 0;
	int len = 0;
	len = strlen(str);
	args[ntokens]= &str[i];/*asigna la direccion de memoria*/
	for (i = 0;i<len;i++){
		if(ntokens <= maxargs){
			eschar(str,&i);
			nochar(str,&i);
			if (str[i] != '\0'){
				args[ntokens] = &str[i];
				ntokens++;
			}
		}
	}
	return ntokens;
}

int
main(int argc, char *argv[])
{
	int n;
	char *args[10];
	char cadena[] = "    Hola  que  mundo \t que \r tal estas que pasa  \n ";
	int i=0;
	n = mytokenize (cadena , args , 10);
	for (i = 0;i<n;i++){
		printf("%d:%s\n",i,args[i]);
	}
	exit(EXIT_SUCCESS);
}
