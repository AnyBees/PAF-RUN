#include "structures.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

int TaskNbr;
int virtualCoreNbr;

int main(int argc, char* argv[]){

	int i;
	int j;
	int nbproc;
	int nbD;
	int nbP;

	DS dualServer[30];
	PS primaryServer[30];

	if (argc != 2) {
		printf("You must give the input file %s\n", argv[0]);
		return 1;
	}

	FILE * fp;

	fp = fopen (strcat(argv[1],".off"), "r");

	if(fp == NULL){
		printf("No such file found (don't write the .off) %s\n", argv[1]);
		return 1;
	}

	fscanf(fp, "Le systeme s'ordonnance sur %d processeurs\n\n", &nbproc);

	fscanf(fp, "number of dual servers = %d\n", &nbD);

	TaskNbr = 0;

	for(i = 0 ; i<nbD ; i++){
		fscanf(fp, "\nbegin D\nnumber = %d\nrate = %d/%d\nnumber of periods = %d\nperiods = ", &dualServer[i].name, &dualServer[i].ratenum, &dualServer[i].rateden, &dualServer[i].number);
		for(j = 0 ; j<dualServer[i].number ; j++)
			fscanf(fp, "%d ", &dualServer[i].periods[j]);
		fscanf(fp, "\nfather = %d\nson = %d\nend D\n", &dualServer[i].father, &dualServer[i].son);
		if(dualServer[i].son == 0)
			TaskNbr ++;
	}

	fscanf(fp, "\nnumber of primary servers = %d\n", &nbP);

	virtualCoreNbr = 0;

	for(i = 0 ; i<nbP ; i++){
		fscanf(fp, "\nbegin P\nnumber = %d\nrate = %d/%d\nfather = %d\nnumber of sons = %d\nsons = ", &primaryServer[i].name, &primaryServer[i].ratenum, &primaryServer[i].rateden, &primaryServer[i].father, &primaryServer[i].size);
		for(j = 0 ; j<primaryServer[i].size ; j++)
			fscanf(fp, "%d ", &primaryServer[i].son[j]);
		fscanf(fp, "\nend P\n");
		if(primaryServer[i].son[0] <= TaskNbr)
			virtualCoreNbr ++;			
	}

	printf("Il y a %d taches sur %d processeurs et %d coeurs virtuels\n", TaskNbr, nbproc, virtualCoreNbr);

	fclose(fp);

	return 0;
}
