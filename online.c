#include "structures.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

void *VCoreExec(void *arg);
void *DualExec(void *arg);
void *TaskExec(void *arg);
void complete(int nbr);
void insertion(int nbr);
int minDeadline(int name);

int TaskNbr;
int nbproc;
int levels;
int fatherCoreNbr;
DS dualServer[30];
PS primaryServer[30];

pthread_cond_t  primary_Var[20];

pthread_mutex_t  primary_Lock[20];

pthread_mutex_t  mainLock;

TSK Tasks[60];

int q = 100000; // Length of a quantum in microseconds

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task;

Shared_Task Shared_T;

pthread_t primaryThreads[30];
pthread_t dualThreads[30];

int main(int argc, char* argv[]){

	int i;
	int j;
	int nbD;
	int nbP;
	int Dlevels[20];
	int Plevels[20];

	/* Getting all datas from .off file */

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

	fatherCoreNbr = 0;

	for(i = 0 ; i<nbP ; i++){
		fscanf(fp, "\nbegin P\nnumber = %d\nrate = %d/%d\nfather = %d\nnumber of sons = %d\nsons = ", &primaryServer[i].name, &primaryServer[i].ratenum, &primaryServer[i].rateden, &primaryServer[i].father, &primaryServer[i].size);
		for(j = 0 ; j<primaryServer[i].size ; j++)
			fscanf(fp, "%d ", &primaryServer[i].son[j]);
		fscanf(fp, "\nend P\n");
		primaryServer[i].active = false;
		primaryServer[i].texec = -1;
		if(primaryServer[i].son[0] < TaskNbr)
			fatherCoreNbr ++;		
	}

	Dlevels[1] = TaskNbr-1;
	Plevels[1] = fatherCoreNbr-1;

	printf("Il y a %d taches sur %d processeurs et %d coeurs virtuels\n", TaskNbr, nbproc, fatherCoreNbr);

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

	for (i = 0 ; i<nbP ; i++){
		pthread_cond_init(&primary_Var[i], NULL);
 		pthread_mutex_init(&primary_Lock[i], NULL);
	}

	pthread_mutex_init(&mainLock, NULL);

	pthread_mutex_lock(&primary_Lock[nbP-1]);

	Tasks[nbP-1+30].next = nbP-1+30;

	for (i = Dlevels[2*(levels-1)]; i <= Dlevels[2*levels-1] ; i++){
		int *arg = malloc(sizeof(*arg));
		*arg = i;
		pthread_create(&dualThreads[i], NULL, DualExec, arg);
		printf("vient d'etre cree (serveur dual) : (0x)%x\n", (int) dualThreads[i]);
	}

	sleep(1);

	pthread_mutex_unlock(&primary_Lock[nbP-1]);

	while(true){
		
		pthread_mutex_lock(&primary_Lock[nbP-1]);	
		while(Tasks[nbP-1+30].next != nbP-1+30){
			pthread_mutex_lock(&mainLock);
			primaryServer[nbP-1].texec ++;
			printf("time = %d\n", primaryServer[nbP-1].texec);
			pthread_mutex_unlock(&mainLock);
			pthread_cond_wait(&primary_Var[nbP-1],&primary_Lock[nbP-1]);
			pthread_mutex_unlock(&primary_Lock[nbP-1]);	
			usleep(1000);
		}
		pthread_mutex_lock(&mainLock);
		primaryServer[nbP-1].texec ++;
		printf("time = %d\n", primaryServer[nbP-1].texec);
		pthread_mutex_unlock(&mainLock);
		usleep(q);
		pthread_cond_broadcast(&primary_Var[nbP-1]);
		pthread_mutex_unlock(&primary_Lock[nbP-1]);
		usleep(nbproc*1000);
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
	return min;
}

void *TaskExec(void *arg){

	int nbr = *((int *) arg);

	int father = dualServer[nbr].father;
	int time;

	int w = dualServer[nbr].ratenum*q;

	Tasks[nbr].next = father;
	Tasks[nbr].previous = father;
	Tasks[nbr].active = true;

	while(true){

		while(w >= 1 && Tasks[nbr].active){

			pthread_mutex_lock(&primary_Lock[father]);

			while(Tasks[father+30].next != nbr){
				//printf("Tâche %d en attente de passer prioritaire\n", nbr);
				pthread_cond_wait(&primary_Var[father],&primary_Lock[father]);
			}

			usleep(q*nbproc);
			printf("Tache %d a consomme un quantum dans le coeur\n", nbr);
			w -= q;
			pthread_cond_wait(&primary_Var[father],&primary_Lock[father]);
			pthread_mutex_unlock(&primary_Lock[father]);
			usleep(1000);
		}

		complete(nbr);

		pthread_mutex_lock(&mainLock);

		time = primaryServer[father].texec;

		pthread_mutex_unlock(&mainLock);

		while((Tasks[nbr].complete)*(dualServer[nbr].periods[0])-time/q > 0){
			usleep(q);
			time = primaryServer[father].texec;
		}

		pthread_mutex_lock(&primary_Lock[father]);
		printf("Tache %d va se reactiver\n", nbr);
		insertion(nbr);
		Tasks[nbr].active = 1;

		w = dualServer[nbr].ratenum*q;

		pthread_mutex_unlock(&primary_Lock[father]);
		usleep(1000);

	}

	return 0;

}

void complete(int nbr){

	int i;
	int father = dualServer[nbr].father;
	int deadline;
	int index;
	int mustUp = 1;

	Tasks[nbr].complete++;
	Tasks[nbr].active = 0;
	Tasks[Tasks[nbr].previous].next = Tasks[nbr].next;
	Tasks[Tasks[nbr].next].previous = Tasks[nbr].previous;
	Tasks[nbr].next = father;
	Tasks[nbr].previous = father;
	
	deadline = minDeadline(nbr);
	for(i = 0 ; i < dualServer[i].number ; i++){
		if(dualServer[nbr].deadlines[i] == deadline)
			break;
	}

	index = i;

	while(mustUp){
		mustUp = 0;
		deadline += dualServer[nbr].periods[index];
		for(i = 0 ; i < dualServer[i].number ; i++){
			if(dualServer[nbr].deadlines[i] == deadline){
				mustUp = 1;
				break;
			}
		}
	}

	dualServer[nbr].deadlines[index] = deadline;

	printf("Tâche %d exécutée %d fois\n", nbr, Tasks[nbr].complete);

}

void insertion(int nbr){

	int inserted = 0;

	int father = dualServer[nbr].father+30;
	int i = father;

	while(Tasks[i].next != father){
		if(Tasks[Tasks[i].next].deadline > Tasks[nbr].deadline){
			Tasks[nbr].next = Tasks[i].next;
			Tasks[i].next = nbr;
			Tasks[nbr].previous = i;
			Tasks[Tasks[nbr].next].previous = nbr;
			inserted = 1;
			printf("Tâche %d ajoutée après %d\n", nbr, Tasks[nbr].previous);
			break;
		}
		else
			i = Tasks[i].next;
	}

	if (!inserted){

		Tasks[nbr].previous = i;
		Tasks[nbr].next = father;
    	Tasks[i].next = nbr;

		printf("Tâche %d ajoutée en dernière position, ie après %d\n", nbr, i);
	}

}

void *VCoreExec(void *arg){

	int i;
	int nbr = *((int *) arg);

	pthread_mutex_lock(&primary_Lock[nbr]);
	Tasks[nbr+30].next = nbr+30;

	usleep(1000);

	for (i = 0; i < primaryServer[nbr].size; i++){
		pthread_create(&dualThreads[primaryServer[nbr].son[i]], NULL, TaskExec, &primaryServer[nbr].son[i]);
		printf("vient d'etre cree (tache) : (0x)%x\n", (int) dualThreads[primaryServer[nbr].son[i]]);
	}

	usleep(1000*nbproc);

	pthread_mutex_unlock(&primary_Lock[nbr]);

	usleep(1000*nbproc);

	while(true){
		pthread_mutex_lock(&primary_Lock[nbr]);	
		while(Tasks[nbr+30].next != nbr+30){
			pthread_cond_wait(&primary_Var[nbr],&primary_Lock[nbr]);	
			usleep(1000);
		}

		usleep(q*nbproc);		
		pthread_mutex_unlock(&primary_Lock[nbr]);
		usleep(nbproc*1000);
	}

	return 0;

}

void *DualExec(void *arg){

	int nbr = *((int *) arg);

	int father = dualServer[nbr].father;
	int time;

	int w = dualServer[nbr].ratenum*minDeadline(nbr)/dualServer[nbr].rateden;
	if(w != 0){

		Tasks[nbr].next = father;
		Tasks[nbr].previous = father;
		Tasks[nbr].active = true;
		Tasks[nbr].deadline = minDeadline(nbr);

		insertion(nbr);

		usleep(1000*nbproc);

		while(true){

			while(w >= 1 && Tasks[nbr].active){

				usleep(3000);
				pthread_mutex_lock(&primary_Lock[father]);

				while(Tasks[father+30].next != nbr){
					//printf("Tâche %d en attente de passer prioritaire\n", nbr);
					pthread_cond_wait(&primary_Var[father],&primary_Lock[father]);
					usleep(2000);
				}

				usleep(q);
				printf("Serveur %d actif\n", nbr);
				w --;				
				pthread_cond_broadcast(&primary_Var[father]);
				pthread_mutex_unlock(&primary_Lock[father]);
			}

			complete(nbr);

			usleep(1000);

			pthread_mutex_lock(&mainLock);

			time = primaryServer[father].texec;

			pthread_mutex_unlock(&mainLock);

			while(time < Tasks[nbr].deadline-1){
				usleep(q);
				time = primaryServer[father].texec;
			}

			pthread_mutex_lock(&primary_Lock[father]);
			printf("Tache %d va se reactiver\n", nbr);

			Tasks[nbr].active = 1;

			w = dualServer[nbr].ratenum*(minDeadline(nbr)-Tasks[nbr].deadline)/dualServer[nbr].rateden;
			printf("w = %d\n", w);

			Tasks[nbr].deadline = minDeadline(nbr);
			insertion(nbr);
			pthread_mutex_unlock(&primary_Lock[father]);
			usleep(1000);

		}
	}

	return 0;

}
