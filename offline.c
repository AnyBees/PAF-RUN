#include "structures.h"
#include <stdio.h>
#include <stdlib.h>

DS dualServer[30];
PS primaryServer[30];

int pack(int first, int last, int pPos);	//  Groups the duals servers and return the created servers number
int dual(int first, int last, int dPos);	// Creat the dual of each primary server 
int reduce(int first, int last, int pPos, int dPos);
int printD(DS server);
int printP(PS server);

int main(int argc, char* argv[]){

	int i;
	int period;
	int res;
	int rate;
	int totalRate;
	int number;
	int pPos;
	int dPos;
	int pCreates;

	PS leaf;
	leaf.name = 0;
	leaf.sons[0] = 0;
	leaf.father = 0;

	DS root;
	root.name = 0;
	root.sons = 0;
	root.father = 0;

	/* Input des tasks */

	if (argc != 2) {
		printf("task number : %s\n", argv[0]);
		return 1;
	}

	number = atoi(argv[1]);

	totalRate = 0;

	for( i = 0 ; i<number ; i++){
		printf("period and rate of use (between 1 and 99) for task number %d ?\n", i+1);
		res = scanf("%d %d", &period, &rate);
		if(res != 2){
			printf("use: period rate !\n");
			return 1;
		}
		if(rate > 99){
			printf("The use rate  must lower than 100 !\n");
			return 1;
		}
		dualServer[i].name = i+1;
		dualServer[i].rate = rate;
		dualServer[i].number = 1;
		totalRate += rate;
		dualServer[i].periods[0] = period;
		dualServer[i].deadlines[0] = period;
		dualServer[i].sons = &leaf;
	}

	if (totalRate % 100 != 0){
		printf("The total use rate isn't a multiple of 100 (%d) \n", totalRate);
		return 1;
	}

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

	for(i = 0 ; i<dPos ; i++){
		printD(dualServer[i]);
	}

	for(i = 0 ; i<pPos ; i++){
		printP(primaryServer[i]);			
	}
	
	return 0;

}

int pack(int first, int last, int pPos){
	int i;
	int j;
	int place;

	/* Initialisation of first DS in a PS */

	int serverNumber = 1;
	primaryServer[pPos].name = 31+pPos;
	primaryServer[pPos].rate = dualServer[first].rate;
	primaryServer[pPos].taille = 1;
	primaryServer[pPos].sons[0] = &(dualServer[first]);
	dualServer[first].father = &(primaryServer[pPos]);

	/* we use first-fit to put the DS in the PS */

	for(i = first+1 ; i < last ; i++){
		place = 0;
		for(j = pPos ; j < pPos+serverNumber ; j++){
			if(primaryServer[j].rate + dualServer[i].rate <= 100){
				primaryServer[j].rate += dualServer[i].rate;
				primaryServer[j].sons[primaryServer[j].taille] = &(dualServer[i]);
				primaryServer[j].taille++;
				dualServer[i].father = &(primaryServer[j]);
				place = 1;
				break;
			}
		}
		if(!place){
			primaryServer[pPos+serverNumber].name = 31+pPos+serverNumber;
			primaryServer[pPos+serverNumber].rate = dualServer[i].rate;
			primaryServer[pPos+serverNumber].taille = 1;
			primaryServer[pPos+serverNumber].sons[0] = &(dualServer[i]);
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
		dualServer[dPos+i].rate = 100-primaryServer[first+i].rate;
		number = 0;
		for(j = 0 ; j < primaryServer[first+i].taille ; j++){
			for(k = 0 ; k < primaryServer[first+i].sons[j]->number ; k++){
				dualServer[dPos+i].periods[number+k] = primaryServer[first+i].sons[j]->periods[k];
				dualServer[dPos+i].deadlines[number+k] = primaryServer[first+i].sons[j]->deadlines[k];
			}
			number += k;
		}
		dualServer[dPos+i].sons = &primaryServer[first+i];
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
	printf("\nserver number %d of rate %d. ", server.name, server.rate);
	if(server.sons->name == 0)
		printf("it is a leaf, meaning 'a real task', its father is %d and its period is %d \n", server.father->name, server.periods[0]);
	else{
		printf("its son is the server number %d, and its father is the server number %d its périods are ", server.sons->name, server.father->name);
		for(i = 0 ; i < server.number ; i++)
			printf("%d ", server.periods[i]);
		printf("\n");
	}
	return 1;
}

int printP(PS server){
	int i;
	printf("\nserver number %d of rate %d. ", server.name, server.rate);
	if(server.father->name == 0)
		printf("it is a root, ");
	else
		printf("its father is the server number %d, ", server.father->name);
	printf("its sons are  number ");
	for(i = 0 ; i < server.taille ; i++)
		printf("%d ", server.sons[i]->name);
	printf("\n");
	return 1;
}

