#include "structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DS dualServer[30];
PS primaryServer[30];

int pack(int first, int last, int pPos);	//  Groups the duals servers in dualServer[first ... last-1] and appends the created primary servers in primaryservers beginning at range pPos. Returns the number of primary servers that were created in this step
int dual(int first, int last, int dPos);	// Creat the dual of each primary server in primaryServer[first ... last-1] and appends the created dual servers in primaryservers beginning at range dPos. Returns the number of dual servers that were created in this step
int reduce(int first, int last, int pPos, int dPos);  // Just applying dual between first and last-1 and packing the resulting dual servers.  Returns the number of primary servers that were created in this step
int printD(DS server);	// To display datas in the console
int printP(PS server);
int gcd(int a, int b);

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
	int tauxTotalNum = 0;
	int tauxTotalDen = 1;
	int sub;
	int nbproc;
	int levels;
	int relevant;
	char out[80];

	/* Getting tasks from input file */

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

	while(res == 2){  // res != 2 when the end of file is reached	
		res = fscanf(fp, "%d %d", &period, &wcet);
		if(wcet > period){
			printf("Worst case execution time must be lower than period\n");
			return 1;
		}
		dualServer[i].name = i;
		dualServer[i].ratenum = wcet;
		dualServer[i].rateden = period; // rate = wcet/period
		dualServer[i].number = 1;
		dualServer[i].periods[0] = period;
		dualServer[i].son = -1; // This server is a task, so a leaf
		if(res == 2){
			tauxTotalNum = tauxTotalNum*period+tauxTotalDen*wcet;
			tauxTotalDen = tauxTotalDen*period;
			sub = gcd(tauxTotalDen, tauxTotalNum);
			tauxTotalNum = tauxTotalNum/sub;
			tauxTotalDen = tauxTotalDen/sub;
		}
		i++;
	}

	number = i-1; // Getting the number of tasks from the number of iterations of the loop

	fclose(fp);

	dPos = number;
	pPos = 0;

	pCreates = pack(0, dPos, pPos); // Packing the tasks
	pPos += pCreates;

	levels = 1;

	while(pCreates > 1){ // when only one primary server is created, then it is the root
		dPos += pCreates;
		pCreates = reduce(pPos-pCreates, pPos, pPos, dPos-pCreates);
		pPos += pCreates;
		levels ++;	// counting the number of iterations to get the tree's size
	}

	primaryServer[pPos-1].father = -1; // this server is the root

	/* Multiplicating periods to keep integers in online part */

	sub = 1;
	
	for(i = number ; i<dPos ; i++){
		relevant = 0;
		for(j = 0 ; j<dualServer[i].number ; j++){
			if(dualServer[i].periods[j] % dualServer[i].rateden != 0){
				relevant = 1;
				break;
			}
		}
		if(relevant)
			sub = sub*dualServer[i].rateden/gcd(sub,dualServer[i].rateden);
	}

	for(i = 0 ; i<dPos ; i++){
		for(j = 0 ; j<dualServer[i].number ; j++)
			dualServer[i].periods[j] = dualServer[i].periods[j]*sub;
		if(i < number){
			dualServer[i].ratenum = dualServer[i].ratenum*sub;
			dualServer[i].rateden = dualServer[i].rateden*sub;
		}
	}

	/* Writing datas to a file that can be read by the online part */

	fp = fopen (strcat(out,".off"), "w");

	if(tauxTotalNum%tauxTotalDen == 0)
		nbproc = tauxTotalNum/tauxTotalDen;
	else
		nbproc = tauxTotalNum/tauxTotalDen + 1;

	fprintf(fp, "Le systeme s'ordonnance sur %d processeurs. L'arbre est obtenu apres %d iterations\n\n", nbproc, levels);

	fprintf(fp, "number of dual servers = %d\n", dPos);

	for(i = 0 ; i<dPos ; i++){
		fprintf(fp, "\nbegin D\nnumber = %d\nrate = %d/%d\nnumber of periods = %d\nperiods = ", dualServer[i].name, dualServer[i].ratenum, dualServer[i].rateden, dualServer[i].number);
		for(j = 0 ; j<dualServer[i].number ; j++)
			fprintf(fp, "%d ", dualServer[i].periods[j]);
		fprintf(fp, "\nfather = %d\nson = %d\nend D\n", dualServer[i].father, dualServer[i].son);
		printD(dualServer[i]);
	}

	fprintf(fp, "\nnumber of primary servers = %d\n", pPos);

	for(i = 0 ; i<pPos ; i++){
		fprintf(fp, "\nbegin P\nnumber = %d\nrate = %d/%d\nfather = %d\nnumber of sons = %d\nsons = ", primaryServer[i].name, primaryServer[i].ratenum, primaryServer[i].rateden, primaryServer[i].father, primaryServer[i].size);
		for(j = 0 ; j<primaryServer[i].size ; j++)
			fprintf(fp, "%d ", primaryServer[i].son[j]);
		fprintf(fp, "\nend P\n");
		printP(primaryServer[i]);	
	}

	fclose(fp);

	printf("On a multiplié l'échelle des temps par %d\n", sub);
	
	return 0;

}

int pack(int first, int last, int pPos){
	int i;
	int j;
	int place;
	int sub;

	/* Initialisation of first DS in a PS */

	int serverNumber = 1;
	primaryServer[pPos].name = pPos;
	primaryServer[pPos].ratenum = dualServer[first].ratenum;
	primaryServer[pPos].rateden = dualServer[first].rateden;
	primaryServer[pPos].size = 1;
	primaryServer[pPos].son[0] = first;
	dualServer[first].father = pPos;

	/* we use first-fit to put the DS in the PS */

	for(i = first+1 ; i < last ; i++){
		place = 0;
		for(j = pPos ; j < pPos+serverNumber ; j++){
			if(primaryServer[j].ratenum*dualServer[i].rateden + dualServer[i].ratenum*primaryServer[j].rateden <= dualServer[i].rateden*primaryServer[j].rateden){
				// As the rate is a fraction, the condition and calculus of the new rate are a little bit complicated
				primaryServer[j].ratenum = primaryServer[j].ratenum*dualServer[i].rateden + dualServer[i].ratenum*primaryServer[j].rateden;
				primaryServer[j].rateden = primaryServer[j].rateden*dualServer[i].rateden;
				sub = gcd(primaryServer[j].rateden, primaryServer[j].ratenum); // simplificating rate to avoid to big integers
				primaryServer[j].rateden = primaryServer[j].rateden/sub;
				primaryServer[j].ratenum = primaryServer[j].ratenum/sub;
				primaryServer[j].son[primaryServer[j].size] = i;
				primaryServer[j].size++;
				dualServer[i].father = j;
				place = 1;
				break;
			}
		}
		if(!place){ // If the dual server found no place in any existing primary server, then we create a new one
			primaryServer[pPos+serverNumber].name = pPos+serverNumber;
			primaryServer[pPos+serverNumber].ratenum = dualServer[i].ratenum;
			primaryServer[pPos+serverNumber].rateden = dualServer[i].rateden;
			primaryServer[pPos+serverNumber].size = 1;
			primaryServer[pPos+serverNumber].son[0] = i;
			dualServer[i].father = pPos+serverNumber;
			serverNumber++;
		}
	}

	return serverNumber;

}
		
int dual(int first, int last, int dPos){
	int i;
	int j;
	int k;
	int l;
	int number;

	for(i = 0 ; i < last-first ; i++){
		dualServer[dPos+i].name = dPos+i;
		dualServer[dPos+i].rateden = primaryServer[first+i].rateden;
		dualServer[dPos+i].ratenum = primaryServer[first+i].rateden-primaryServer[first+i].ratenum;
		number = 1;
		dualServer[dPos+i].periods[0] = dualServer[primaryServer[first+i].son[0]].periods[0];
		for(j = 0 ; j < primaryServer[first+i].size ; j++){ // loop to get all the grandsons
			for(k = 0 ; k < dualServer[primaryServer[first+i].son[j]].number ; k++){ // get all their periods
				for(l = 0 ; l < number ; l++){ // comparing to already got periods to check if it is relevant to add
					if(dualServer[primaryServer[first+i].son[j]].periods[k] % dualServer[dPos+i].periods[l] == 0)
						break;
					if(dualServer[dPos+i].periods[l] % dualServer[primaryServer[first+i].son[j]].periods[k] == 0){
						dualServer[dPos+i].periods[l] = dualServer[primaryServer[first+i].son[j]].periods[k];
						break;
					}
					dualServer[dPos+i].periods[number] = dualServer[primaryServer[first+i].son[j]].periods[k];
					number++;
				}				
			}
		}
		dualServer[dPos+i].son = first+i;
		primaryServer[first+i].father = dPos+i;
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
	if(server.son == -1)
		printf("it is a leaf, meaning 'a real task', its father is %d and its period is %d \n", server.father, server.periods[0]);
	else{
		printf("his son is the server number %d, and his father is the server number %d its periods are ", server.son, server.father);
		for(i = 0 ; i < server.number ; i++)
			printf("%d ", server.periods[i]);
		printf("\n");
	}
	return 1;
}

int printP(PS server){
	int i;
	printf("\nserver number %d of rate %d/%d. ", server.name, server.ratenum, server.rateden);
	if(server.father == -1)
		printf("it is the root, ");
	else
		printf("his father is the server number %d, ", server.father);
	printf("his sons are number ");
	for(i = 0 ; i < server.size ; i++)
		printf("%d ", server.son[i]);
	printf("\n");
	return 1;
}
/* find the greatest common divisor*/ 
int gcd(int c, int d){
	int a;
	int b;
	int r;
	if(c>d){
		a = c;
		b = d;
	}
	else{
		a = d;
		b = c;
	}
	if(b==0)
		return a;
	r= a % b; 
	while(r!=0){
		a = b; 
		b = r; 
		r = a % b; 
	}
	return b;
}
