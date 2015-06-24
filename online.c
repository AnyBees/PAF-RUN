#include "structures.h"
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>
#include <string.h>

void *VCoreExec(void *i);
void *TaskExec(void *i);
void complete(int nbr);
void insertion(int nbr);
int minDeadline(DS server);

int TaskNbr;
int virtualCoreNbr;
int virtualCores[20];

int main(int argc, char* argv[]){

	int i;
	int j;
	int k;
	int num;
	int min;
	int nbproc;
	int nbD;
	int nbP;
	int levels;
	int Dlevels[20];
	int Plevels[20];
	int cores[20];

	DS dualServer[30];
	PS primaryServer[30];

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
		for(j = 0 ; j<dualServer[i].number ; j++)
			fscanf(fp, "%d ", &dualServer[i].periods[j]);
			dualServer[i].deadlines[j] = dualServer[i].periods[j];
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

	primaryServer[nbP-1].active = true; //always active

	for (i = 0 ; i<nbproc ; i++)
		cores[i] = -1;

	for (i = 0 ; i<virtualCoreNbr ; i++)
		virtualCores[i] = -1;

	/* Assiociating real cores with virtual ones */

	for(i = 0 ; i<nbD ; i++)
		dualServer[i].active = false;

	for(i = 0 ; i<nbP-1 ; i++)
		dualServer[i].active = false;

	for(i = levels-1 ; i>0 ; i--){
		for (j = Plevels[2*i] ; j < Plevels[2*i+1]+1 ; j++){
			if(primaryServer[j].active){
				num = primaryServer[j].son[0];
				min = minDeadline(dualServer[primaryServer[j].son[0]]);
				for (k = 1 ; k < primaryServer[j].size ; k++){
					if(minDeadline(dualServer[primaryServer[j].son[k]]) < min)
						num = primaryServer[j].son[k];
						min = minDeadline(dualServer[primaryServer[j].son[k]]);
				}
				dualServer[num].active = true;
			}
		}
		for (j = Plevels[2*(i-1)] ; j < Plevels[2*i-1]+1 ; j++){
			if (!dualServer[primaryServer[j].father].active)
				primaryServer[j].active = true;
		}
	}

	for(i = 0 ; i<nbD ; i++){
		if(dualServer[i].active)
			printf("%d ", i);
	}
	printf("duaux activés \n");	

	for(i = 0 ; i<nbP ; i++){
		if(primaryServer[i].active)
			printf("%d ", i);
	}
	printf("primaires activés \n");

	for(i = 0 ; i<virtualCoreNbr ; i++){
		if(primaryServer[i].active){
			if(virtualCores[i] == -1){
				for (j = 0 ; j<nbproc ; j++){
					if(!primaryServer[cores[j]].active){
						virtualCores[cores[j]] = -1;
						cores[j] = i;
						virtualCores[i] = j;
						break;
					}
				}
			}
		}
	}

	for(i = 0 ; i<virtualCoreNbr ; i++)
		printf("%d ", virtualCores[i]);
	printf("pour les cv\n");

	for(i = 0 ; i<nbproc ; i++)
		printf("%d ", cores[i]);
	printf("pour les coeurs\n");

	return 0;
}

int minDeadline(DS server){
	int i;
	int min = server.deadlines[0];
	for (i = 1 ; i<server.number ; i++){
		if(server.deadlines[i] < min)
			min = server.deadlines[i];
	}
	return min;
}

void *TaskExec(void *i){

  int nbr = *((int *) i);

  Tasks[nbr].active = 1;

  float w = Tasks[nbr].WCET;
	printf("w%d = %f\n", nbr, w);
  float q = 1000000;

  insertion(nbr);

	while(true){

		while(w >= q && Tasks[nbr].active){

			pthread_mutex_lock(&Shared_T.Lock);

			while(Tasks[0].next != nbr){
				printf("Tâche %d en attente de passer prioritaire\n", nbr);
				pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
			}

		  usleep(q);
			printf("Tache %d a consomme un quantum\n", nbr);
		  w -= q;

		  texec += q;
			pthread_mutex_unlock(&Shared_T.Lock);
			usleep(1000);
		}

	  complete(nbr);

		printf("Sleep %f\n",(Tasks[nbr].complete)*(Tasks[nbr].period)-texec);

		usleep((Tasks[nbr].complete)*(Tasks[nbr].period)-texec);

	  pthread_mutex_lock(&Shared_T.Lock);
    insertion(nbr);
    Tasks[nbr].active = 1;
		w = Tasks[nbr].WCET;
		printf("Tache %d va se reactiver\n", nbr);

    printf("texec = %d\n", texec);
		pthread_mutex_unlock(&Shared_T.Lock);

  }

  //free(i);

return 0;

}

void complete(int nbr){

  pthread_cond_broadcast(&Shared_T.Cond_Var);

  Tasks[nbr].deadline += Tasks[nbr].period;

  Tasks[nbr].complete++;
  Tasks[nbr].active = 0;
	Tasks[Tasks[nbr].previous].next = Tasks[nbr].next;
	Tasks[Tasks[nbr].next].previous = Tasks[nbr].previous;
	Tasks[nbr].next = 0;
	Tasks[nbr].previous = 0;

  printf("Tâche %d exécutée %d fois\n", nbr, Tasks[nbr].complete);

}

void insertion(int nbr){

  int i=0;
	int inserted = 0;

  /*if (*lTasks == NULL || *Tasks[nbr]  == NULL){
    exit(EXIT_FAILURE);
  }*/

	while(Tasks[i].next != 0){
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
		Tasks[nbr].next = 0;
    Tasks[i].next = nbr;

		printf("Tâche %d ajoutée en dernière position, ie après %d\n", nbr, i);
	}

}

void *VCoreExec(void *i){

  int i;
  int nbr;
  float wcet;
  float period;
  float totalrate = 0;

  if (argc != 2){
    printf("Usage : %s nombre-de-taches\n", argv[0]);
    exit(1);
  }

  TaskNbr = atoi(argv[1]);

  printf("Quels sont le WCET et la periode pour chaque tache ?\n");

  Tasks[0].number = 0;
	Tasks[0].next = 0;

  for (i = 1; i <= TaskNbr; i++){
    nbr = scanf("%f %f", &wcet, &period);
    if (nbr != 2){
      printf("L'usage est WCET-tache periode-tache\n");
    }

    Tasks[i].number = i;
    Tasks[i].period = period*1000000;
    Tasks[i].deadline = period*1000000;
    Tasks[i].WCET = wcet*1000000;
    Tasks[i].complete = 0;
		Tasks[i].next = 0;
  }

  for (i = 1; i <= TaskNbr; i++){
    totalrate += (Tasks[i].WCET/Tasks[i].period)*100;
  }

  if (totalrate > 100){
    printf("Le taux d'utilisation du CPU ne peut etre superieur a 100 pourcents\n");
  }

  pthread_t Threads[TaskNbr];

  /* Initialisation des vars. partagees */

  pthread_cond_init(&Shared_T.Cond_Var, NULL);
  pthread_mutex_init(&Shared_T.Lock, NULL);

  pthread_mutex_lock(&Shared_T.Lock);

  for (i = 1; i <= TaskNbr; i++){
    int *arg = malloc(sizeof(*arg));
    *arg = i;
    pthread_create(&Threads[i], NULL, TaskExec, arg);
    printf("vient d'etre cree : (0x)%x\n", (int) Threads[i]);
  }

  sleep(1);

  pthread_mutex_unlock(&Shared_T.Lock);

	sleep(1);

	pthread_mutex_lock(&Shared_T.Lock);

	while(true){	
		while(Tasks[0].next != 0){
			pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
		}

		usleep(1000000);
		printf("Processor is idle\n");

		texec += 1000000;
		pthread_mutex_unlock(&Shared_T.Lock);
		usleep(1000);
	}

	return 0;

}
