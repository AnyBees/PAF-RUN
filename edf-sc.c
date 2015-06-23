#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "structures.h"
#include <stdbool.h>

#define MAX_TASKS 8

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task;

Shared_Task Shared_T;

TSK Tasks[MAX_TASKS];

void *TaskExec(void *i);
void activate(int nbr);
void proceed(int nbr);
void complete(int nbr);
lTSK *initialisation(int nbr);
void insertion(lTSK *lTasks, int nbr);

lTSK lTasks;

int TaskNbr;
int texec = 0;

int main (int argc, char *argv[]){

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
    Tasks[i].period = period*1000;
    Tasks[i].deadline = period*1000;
    Tasks[i].WCET = wcet*1000;
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

  sleep(2);

  pthread_mutex_unlock(&Shared_T.Lock);

	sleep(10);

	return 0;

}

void *TaskExec(void *i){

  int nbr = *((int *) i);
  int k;

  Tasks[nbr].active = 1;

  while(1){

  float w = Tasks[nbr].WCET;
	printf("w%d = %f\n", nbr, w);
  float q = 1000;

  activate(nbr);

  while(w >= q && !Tasks[nbr].complete){

    proceed(nbr);
    usleep(q);
		printf("Tache %d a consomme un quantum\n", nbr);
    w -= q;

    texec += q;

    for (k = 1; k <= TaskNbr; k++){
      if (k != nbr){
        if (texec >= (Tasks[k].complete)*(Tasks[k].period)){
          Tasks[k].active = 1;
          activate(k);
        }
      }
    }

    printf("texec = %d\n", texec);

  }

  complete(nbr);

  //free(i);

return 0;

}

}

void activate (int nbr){

  lTSK lTsk;

  lTSK *lTasks = malloc(sizeof(lTSK));

  if (lTasks->first == NULL){
    lTsk = *initialisation(nbr);
  }

  insertion(&lTsk, nbr);

}

void proceed(int nbr){

  pthread_mutex_lock(&Shared_T.Lock);

  while(Tasks[0].next != &Tasks[nbr]){
    printf("Tâche %d en attente de passer prioritaire\n", nbr);
    pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
  }

  pthread_mutex_unlock(&Shared_T.Lock);

}

void complete(int nbr){

  pthread_cond_broadcast(&Shared_T.Cond_Var);

  Tasks[nbr].deadline += Tasks[nbr].period;

  Tasks[nbr].complete++;
  Tasks[nbr].active = 0;
	Tasks[nbr].previous->next =Tasks[nbr].next;
	Tasks[nbr].next->previous =Tasks[nbr].previous;
	Tasks[nbr].next = 0;
	Tasks[nbr].previous = 0;

  printf("Tâche %d exécutée %d fois\n", nbr, Tasks[nbr].complete);

}

lTSK *initialisation(int nbr){

  /*if (*lTasks == NULL || *Tasks[nbr] == NULL){
     exit(EXIT_FAILURE);
  }*/
  Tasks[nbr].next = NULL;
  Tasks[nbr].previous = NULL;
  lTasks.first = &Tasks[nbr];
  return &lTasks;

}

void insertion(lTSK *lTasks, int nbr){

  int i=0;
	int inserted = 0;

  /*if (*lTasks == NULL || *Tasks[nbr]  == NULL){
    exit(EXIT_FAILURE);
  }*/

	while(Tasks[i].next != 0){
		if(Tasks[i].next->deadline > Tasks[nbr].deadline){
			Tasks[nbr].next = Tasks[i].next;
			Tasks[i].next = &Tasks[nbr];
			Tasks[nbr].previous = &Tasks[i];
			Tasks[nbr].next->previous = &Tasks[nbr];
			inserted = 1;
			printf("Tâche %d ajoutée après %d\n", nbr, (Tasks[nbr].previous)->number);
			break;
		}
		else
			i = Tasks[i].next->number;
	}

	if (!inserted){

		Tasks[nbr].previous = &Tasks[i];
    Tasks[i].next = &Tasks[nbr];

		printf("Tâche %d ajoutée en dernière position, ie après %d\n", nbr, i);
	}

}
