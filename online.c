#include "structures.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

#define MAX_INT 2147483647

void *VCoreExec(void *arg);
void *TaskExec(void *arg);
void complete(int nbr);
void insertion(int nbr);
int minDeadline(int name);

int TaskNbr;
int nbproc;
int levels;
int virtualCoreNbr;
int virtualCores[20];
DS dualServer[30];
PS primaryServer[30];

pthread_cond_t  proc_Var[20];

pthread_mutex_t  proc_Lock[20];

pthread_mutex_t  mainLock;

TSK Tasks[60];

int q = 1000000; // Length of a quantum in microseconds

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task;

Shared_Task Shared_T;

pthread_t vcThreads[20];
pthread_t taskThreads[30];

int main(int argc, char* argv[]){

	int i;
	int j;
	int k;
	int num;
	int min;
	int nbD;
	int nbP;
	int Dlevels[20];
	int Plevels[20];
	int cores[20];

	/* Getting all datas from .off file */

	printf("max int = %d\n", MAX_INT);

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

	fscanf(fp, "Le systeme s'ordonnance sur %d processeurs. L'arbre est obtenu apres %d iterations\n\n", &nbproc, &levels);

	fscanf(fp, "number of dual servers = %d\n", &nbD);

	Dlevels[0] = 0;
	Plevels[0] = 0;

	TaskNbr = 0;

	for(i = 0 ; i<nbD ; i++){
		fscanf(fp, "\nbegin D\nnumber = %d\nrate = %d/%d\nnumber of periods = %d\nperiods = ", &dualServer[i].name, &dualServer[i].ratenum, &dualServer[i].rateden, &dualServer[i].number);
		for(j = 0 ; j<dualServer[i].number ; j++){
			fscanf(fp, "%d ", &dualServer[i].periods[j]);
			dualServer[i].deadlines[j] = dualServer[i].periods[j];
		}
		fscanf(fp, "\nfather = %d\nson = %d\nend D\n", &dualServer[i].father, &dualServer[i].son);
		dualServer[i].active = false;
		if(dualServer[i].son == -1)
			TaskNbr ++;
	}

	fscanf(fp, "\nnumber of primary servers = %d\n", &nbP);

	virtualCoreNbr = 0;

	for(i = 0 ; i<nbP ; i++){
		fscanf(fp, "\nbegin P\nnumber = %d\nrate = %d/%d\nfather = %d\nnumber of sons = %d\nsons = ", &primaryServer[i].name, &primaryServer[i].ratenum, &primaryServer[i].rateden, &primaryServer[i].father, &primaryServer[i].size);
		for(j = 0 ; j<primaryServer[i].size ; j++)
			fscanf(fp, "%d ", &primaryServer[i].son[j]);
		fscanf(fp, "\nend P\n");
		primaryServer[i].active = false;
		primaryServer[i].texec = -q;
		if(primaryServer[i].son[0] < TaskNbr)
			virtualCoreNbr ++;		
	}

	Dlevels[1] = TaskNbr-1;
	Plevels[1] = virtualCoreNbr-1;

	printf("Il y a %d taches sur %d processeurs et %d coeurs virtuels\n", TaskNbr, nbproc, virtualCoreNbr);

	fclose(fp);

	for(i = 1 ; i<levels ; i++){
		Dlevels[2*i] = Dlevels[2*i-1] + 1;
		Plevels[2*i] = Plevels[2*i-1] + 1;
		j = 0;
		while(dualServer[j].son < Plevels[2*i] && j < nbD)
			j ++;
		Dlevels[2*i+1] = j-1;
		j = 0;
		while(primaryServer[j].son[0] <= Dlevels[2*i+1] && j < nbP)
			j ++;
		Plevels[2*i+1] = j-1;
	}

	for (i = 0 ; i<nbproc ; i++){
		cores[i] = -1;
		pthread_cond_init(&proc_Var[i], NULL);
 		pthread_mutex_init(&proc_Lock[i], NULL);
	}

	pthread_mutex_init(&mainLock, NULL);

	for (i = 0; i<virtualCoreNbr ; i++){
		int *arg = malloc(sizeof(*arg));
		*arg = i;
		virtualCores[i] = -1;
		pthread_create(&vcThreads[i], NULL, VCoreExec, arg);
		printf("vient d'etre cree (coeur virtuel) : (0x)%x\n", (int) vcThreads[i]);
	}

	primaryServer[nbP-1].active = true; // root is always active

	/* Associating real cores with virtual ones */

	while(true){

		pthread_mutex_lock(&mainLock);
		for (i = 0; i<virtualCoreNbr ; i++){
			primaryServer[i].texec += q;
		}
		printf("texec = %d\n", primaryServer[0].texec/q);

		for(i = 0 ; i<nbD ; i++)
			dualServer[i].active = false;

		for(i = 0 ; i<nbP-1 ; i++)
			dualServer[i].active = false;

		for(i = levels-1 ; i>0 ; i--){
			for (j = Plevels[2*i] ; j < Plevels[2*i+1]+1 ; j++){
				if(primaryServer[j].active){
					num = -1;
					min = MAX_INT;
					for (k = 0 ; k < primaryServer[j].size ; k++){
						if(minDeadline(primaryServer[j].son[k]) < min && dualServer[primaryServer[j].son[k]].ratenum != 0){
							num = primaryServer[j].son[k];
							min = minDeadline(primaryServer[j].son[k]);
						}
					}
					printf("min = %d (%d)\n", min, num);
					for(k = 0 ; k < dualServer[num].number ; k++)
						printf("%d ",dualServer[num].deadlines[k]);
					printf("deadlines du min\n");
					if(num != -1)
						dualServer[num].active = true;
				}
			}
			for (j = Plevels[2*(i-1)] ; j < Plevels[2*i-1]+1 ; j++){
				if (!dualServer[primaryServer[j].father].active)
					primaryServer[j].active = true;
			}
		}

		for(i = 0 ; i<virtualCoreNbr ; i++){
			if(primaryServer[i].active){
				if(virtualCores[i] == -1){
					for (j = 0 ; j<nbproc ; j++){
						if(!(primaryServer[cores[j]].active)){
							virtualCores[cores[j]] = -1;
							cores[j] = i;
							virtualCores[i] = j;
							break;
						}
					}
				}
			}
		}

		for(i = 0 ; i<virtualCoreNbr ; i++){
			if(primaryServer[i].active){
				pthread_cond_broadcast(&proc_Var[i]);
				printf("%d activé sur coeur %d\n", i, virtualCores[i]);
				//usleep(2*q);				
			}
		}

		pthread_mutex_unlock(&mainLock);
				
		usleep(2*nbproc*q);
	}

	return 0;
}

int minDeadline(int name){
	int i;
	DS server = dualServer[name];
	int min = server.deadlines[0];
	for (i = 0 ; i<server.number ; i++){
		if(server.deadlines[i] < min)
			min = server.deadlines[i];
	}
	printf("deadlines\n");
	return min;
}

void *TaskExec(void *arg){

	int nbr = *((int *) arg);

	int virtual = dualServer[nbr].father;
	int time;
	DS server;
	int i;

	int w = dualServer[nbr].ratenum*q;

	Tasks[nbr].next = virtual;
	Tasks[nbr].previous = virtual;
	Tasks[nbr].active = true;

	insertion(nbr);

	while(true){

		while(w >= q && Tasks[nbr].active){

			pthread_mutex_lock(&proc_Lock[virtual]);

			while(Tasks[virtual+30].next != nbr){
				//printf("Tâche %d en attente de passer prioritaire\n", nbr);
				pthread_cond_wait(&proc_Var[virtual],&proc_Lock[virtual]);
			}

			usleep(q*nbproc);
			printf("Tache %d a consomme un quantum dans le coeur %d\n", nbr, virtualCores[virtual]);
			w -= q;
			pthread_cond_wait(&proc_Var[virtual],&proc_Lock[virtual]);
			pthread_mutex_unlock(&proc_Lock[virtual]);
			usleep(1000);
		}

		complete(nbr);

		pthread_mutex_lock(&mainLock);

		time = primaryServer[virtual].texec;

		pthread_mutex_unlock(&mainLock);

		while((Tasks[nbr].complete)*(dualServer[nbr].periods[0])-time/q > 0){
			usleep(q);
			time = primaryServer[virtual].texec;
		}

		pthread_mutex_lock(&proc_Lock[virtual]);
		printf("Tache %d va se reactiver\n", nbr);
		insertion(nbr);
		Tasks[nbr].active = 1;
		server = dualServer[nbr];

		pthread_mutex_lock(&mainLock);
		while (primaryServer[server.father].father != -1){
			server = dualServer[primaryServer[server.father].father];
			for (i=0 ; i < server.number ; i++){
				if(server.deadlines[i] == MAX_INT){
					server.deadlines[i] = dualServer[nbr].deadlines[0];
					break;
				}
			}
		}
		pthread_mutex_unlock(&mainLock);
		w = dualServer[nbr].ratenum*q;

		pthread_mutex_unlock(&proc_Lock[virtual]);
		usleep(1000);

	}

	return 0;

}

void complete(int nbr){

	int i;
	int virtual = dualServer[nbr].father;
	DS server = dualServer[nbr];

	pthread_cond_broadcast(&proc_Var[virtual]);

	pthread_mutex_lock(&mainLock);

	while (primaryServer[server.father].father != -1){
		server = dualServer[primaryServer[server.father].father];
		for (i=0 ; i < server.number ; i++){
			if(server.deadlines[i] == dualServer[nbr].deadlines[0]){
				server.deadlines[i] = MAX_INT;
				break;
			}
		}
	}

	pthread_mutex_unlock(&mainLock);

	dualServer[nbr].deadlines[0] += dualServer[nbr].periods[0];

	Tasks[nbr].complete++;
	Tasks[nbr].active = 0;
	Tasks[Tasks[nbr].previous].next = Tasks[nbr].next;
	Tasks[Tasks[nbr].next].previous = Tasks[nbr].previous;
	Tasks[nbr].next = virtual;
	Tasks[nbr].previous = virtual;

	printf("Tâche %d exécutée %d fois\n", nbr, Tasks[nbr].complete);

}

void insertion(int nbr){

	int inserted = 0;

	int virtual = dualServer[nbr].father+30;
	int i=virtual;

	while(Tasks[i].next != virtual){
		if(Tasks[Tasks[i].next].deadline > Tasks[nbr].deadline){
			Tasks[nbr].next = Tasks[i].next;
			Tasks[i].next = nbr;
			Tasks[nbr].previous = i;
			Tasks[Tasks[nbr].next].previous = nbr;
			inserted = 1;
			//printf("Tâche %d ajoutée après %d\n", nbr, Tasks[nbr].previous);
			break;
		}
		else
			i = Tasks[i].next;
	}

	if (!inserted){

		Tasks[nbr].previous = i;
		Tasks[nbr].next = virtual;
    	Tasks[i].next = nbr;

		//printf("Tâche %d ajoutée en dernière position, ie après %d\n", nbr, i);
	}

}

void *VCoreExec(void *arg){

	int i;
	int nbr = *((int *) arg);

	pthread_mutex_lock(&proc_Lock[nbr]);
	Tasks[nbr+30].next = nbr+30;

	usleep(1000);

	while(virtualCores[nbr] == -1){
		pthread_cond_wait(&proc_Var[nbr],&proc_Lock[nbr]);
	}

	for (i = 0; i < primaryServer[nbr].size; i++){
		pthread_create(&taskThreads[primaryServer[nbr].son[i]], NULL, TaskExec, &primaryServer[nbr].son[i]);
		printf("vient d'etre cree (tache) : (0x)%x\n", (int) taskThreads[primaryServer[nbr].son[i]]);
	}

	usleep(1000*nbproc);

	pthread_mutex_unlock(&proc_Lock[nbr]);

	usleep(1000*nbproc);

	while(true){
		pthread_mutex_lock(&proc_Lock[nbr]);	
		while(Tasks[nbr+30].next != nbr+30){
			pthread_cond_wait(&proc_Var[nbr],&proc_Lock[nbr]);
			pthread_mutex_unlock(&proc_Lock[nbr]);	
			usleep(nbproc*1000);
			pthread_mutex_lock(&proc_Lock[nbr]);
		}

		usleep(q*nbproc);
		printf("Core %d is idle %d\n", virtualCores[nbr], nbr);

		pthread_mutex_unlock(&proc_Lock[nbr]);
		usleep(nbproc*1000);
	}

	return 0;

	}
