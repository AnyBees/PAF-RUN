#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DS dualServer[30];
PS primaryServer[30];

int pack(int first, int last, int pPos);	//  Groups the duals servers in dualServer[first ... last-1] and appends the created primary servers in primaryservers beginning at range pPos
int dual(int first, int last, int dPos);	// Creat the dual of each primary server in primaryServer[first ... last-1] and appends the created dual servers in primaryservers beginning at range dPos
int reduce(int first, int last, int pPos, int dPos);
int printD(DS server);
int printP(PS server);
int pgcd(int a, int b);

int main(int argc, char* argv[]){

	int i;
	int j;
	int period;
	int res;
	int wcet;
	int number;
	int pPos;
	int dPos;
	int pCreates;
	int sub;
	char out[80];

	PS leaf;
	leaf.name = 0;
	leaf.son[0] = 0;
	leaf.father = 0;

	DS root;
	root.name = 0;
	root.son = 0;
	root.father = 0;

	/* Input des tasks */

	if (argc != 2) {
		printf("You must give the input file %s\n", argv[0]);
		return 1;
	}
	
	strcpy(out, argv[1]);

	FILE * fp;

	fp = fopen (strcat(argv[1],".cfg"), "r");

	if(fp == NULL){
		printf("No such file found (don't write the .cfg) %s\n", argv[1]);
		return 1;
	}

	i = 0;
	res = 2;

	while(res == 2){		
		res = fscanf(fp, "%d %d", &period, &wcet);
		if(wcet > period){
			printf("Worst case execution tome must be lower than periodn");
			return 1;
		}
		sub = pgcd(period, wcet);
		dualServer[i].name = i+1;
		dualServer[i].ratenum = wcet/sub;
		dualServer[i].rateden = period/sub;
		dualServer[i].number = 1;
		dualServer[i].periods[0] = period;
		dualServer[i].deadlines[0] = period;
		dualServer[i].son = &leaf;
		i++;
	}

	number = i-1;

	fclose(fp);

	/*if (totalRate % 100 != 0){
		printf("The total use rate isn't a multiple of 100 (%d) \n", totalRate);
		return 1;
	}*/

	dPos = number;
	pPos = 0;

	pCreates = pack(0, dPos, pPos);
	pPos += pCreates;

	while(pCreates > 1){
		dPos += pCreates;
		pCreates = reduce(pPos-pCreates, pPos, pPos, dPos-pCreates);
		pPos += pCreates;		
	}

	primaryServer[pPos-1].father = &root;

	fp = fopen (strcat(out,".off"), "w");

	fprintf(fp, "number of dual servers = %d\n", dPos);

	for(i = 0 ; i<dPos ; i++){
		fprintf(fp, "\nbegin D\nnumber = %d\nrate = %d/%d\nnumber of periods = %d\nperiods = ", dualServer[i].name, dualServer[i].ratenum, dualServer[i].rateden, dualServer[i].number);
		for(j = 0 ; j<dualServer[i].number ; j++)
			fprintf(fp, "%d ", dualServer[i].periods[j]);
		fprintf(fp, "\nfather = %d\nson = %d\nend D\n", dualServer[i].father->name, dualServer[i].son->name);
		printD(dualServer[i]);
	}

	fprintf(fp, "\nnumber of primary servers = %d\n", pPos);

	for(i = 0 ; i<pPos ; i++){
		fprintf(fp, "\nbegin P\nnumber = %d\nrate = %d/%d\nfather = %d\nnumber of sons = %d\nsons = ", primaryServer[i].name, primaryServer[i].ratenum, primaryServer[i].rateden, primaryServer[i].father->name, primaryServer[i].size);
		for(j = 0 ; j<primaryServer[i].size ; j++)
			fprintf(fp, "%d ", primaryServer[i].son[j]->name);
		fprintf(fp, "\nend P\n");
		printP(primaryServer[i]);			
	}

	fclose(fp);
	
	return 0;

}

int pack(int first, int last, int pPos){
	int i;
	int j;
	int place;
	int sub;

	/* Initialisation of first DS in a PS */

	int serverNumber = 1;
	primaryServer[pPos].name = 31+pPos;
	primaryServer[pPos].ratenum = dualServer[first].ratenum;
	primaryServer[pPos].rateden = dualServer[first].rateden;
	primaryServer[pPos].size = 1;
	primaryServer[pPos].son[0] = &(dualServer[first]);
	dualServer[first].father = &(primaryServer[pPos]);

	/* we use first-fit to put the DS in the PS */

	for(i = first+1 ; i < last ; i++){
		place = 0;
		for(j = pPos ; j < pPos+serverNumber ; j++){
			if(primaryServer[j].ratenum*dualServer[i].rateden + dualServer[i].ratenum*primaryServer[j].rateden <= dualServer[i].rateden*primaryServer[j].rateden){
				primaryServer[j].ratenum = primaryServer[j].ratenum*dualServer[i].rateden + dualServer[i].ratenum*primaryServer[j].rateden;
				primaryServer[j].rateden = primaryServer[j].rateden*dualServer[i].rateden;
				sub = pgcd(primaryServer[j].rateden, primaryServer[j].ratenum);
				primaryServer[j].rateden = primaryServer[j].rateden/sub;
				primaryServer[j].ratenum = primaryServer[j].ratenum/sub;
				primaryServer[j].son[primaryServer[j].size] = &(dualServer[i]);
				primaryServer[j].size++;
				dualServer[i].father = &(primaryServer[j]);
				place = 1;
				break;
			}
		}
		if(!place){
			primaryServer[pPos+serverNumber].name = 31+pPos+serverNumber;
			primaryServer[pPos+serverNumber].ratenum = dualServer[i].ratenum;
			primaryServer[pPos+serverNumber].rateden = dualServer[i].rateden;
			primaryServer[pPos+serverNumber].size = 1;
			primaryServer[pPos+serverNumber].son[0] = &(dualServer[i]);
			dualServer[i].father = &(primaryServer[pPos+serverNumber]);
			serverNumber++;
		}
	}

	return serverNumber;

}
		
int dual(int first, int last, int dPos){
	int i;
	int j;
	int k;
	int number;

	for(i = 0 ; i < last-first ; i++){
		dualServer[dPos+i].name = dPos+i+1;
		dualServer[dPos+i].rateden = primaryServer[first+i].rateden;
		dualServer[dPos+i].ratenum = primaryServer[first+i].rateden-primaryServer[first+i].ratenum;
		number = 0;
		for(j = 0 ; j < primaryServer[first+i].size ; j++){
			for(k = 0 ; k < primaryServer[first+i].son[j]->number ; k++){
				dualServer[dPos+i].periods[number+k] = primaryServer[first+i].son[j]->periods[k];
				dualServer[dPos+i].deadlines[number+k] = primaryServer[first+i].son[j]->deadlines[k];
			}
			number += k;
		}
		dualServer[dPos+i].son = &primaryServer[first+i];
		primaryServer[first+i].father = &dualServer[dPos+i];
		dualServer[dPos+i].number = number;
	}

	return 1;

}

int reduce(int first, int last, int pPos, int dPos){
	dual(first, last, dPos);
	return pack(dPos, dPos+last-first, pPos);
}

int printD(DS server){
	int i;
	printf("\nserver number %d of rate %d/%d. ", server.name, server.ratenum, server.rateden);
	if(server.son->name == 0)
		printf("it is a leaf, meaning 'a real task', its father is %d and its period is %d \n", server.father->name, server.periods[0]);
	else{
		printf("his son is the server number %d, and his father is the server number %d its periods are ", server.son->name, server.father->name);
		for(i = 0 ; i < server.number ; i++)
			printf("%d ", server.periods[i]);
		printf("\n");
	}
	return 1;
}

int printP(PS server){
	int i;
	printf("\nserver number %d of rate %d/%d. ", server.name, server.ratenum, server.rateden);
	if(server.father->name == 0)
		printf("it is a root, ");
	else
		printf("his father is the server number %d, ", server.father->name);
	printf("his sons are number ");
	for(i = 0 ; i < server.size ; i++)
		printf("%d ", server.son[i]->name);
	printf("\n");
	return 1;
}

int pgcd(int a, int b){
	int r;
	r= a % b; 
	while(r!=0){
		a = b; 
		b = r; 
		r = a % b; 
	}
	return b;
}
