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

typedef struct {
  float deadline;
  int number;
} arguments_T;

Shared_Task Shared_T;

arguments_T arguments_Task;

TSK Tasks[MAX_TASKS];

void TaskExec(float d, int nbr);
void activate(int nbr);
void proceed(int nbr);
void complete(int nbr);
lTSK *initialisation(int nbr);
void insertion(lTSK *lTasks, int nbr);

lTSK lTasks;

int main (int argc, char *argv[]){

  int i;
  int nbr;
  float wcet;
  float period;
  float totalrate = 0;
  int TaskNbr;

  if (argc != 2){
    printf("Usage : %s nombre-de-taches\n", argv[0]);
    exit(1);
  }

  TaskNbr = atoi(argv[1]);

  printf("Quels sont le WCET et la periode pour chaque tache ?\n");

  for (i = 0; i < TaskNbr; i++){
    nbr = scanf("%f %f", &wcet, &period);
    if (nbr != 2){
      printf("L'usage est WCET-tache periode-tache\n");
    }

    Tasks[i].number = i+1;
    Tasks[i].period = period*1000;
    Tasks[i].deadline = period*1000;
    Tasks[i].WCET = wcet*1000;
    Tasks[i].complete = 0;
  }

  for (i = 0; i < TaskNbr; i++){
    totalrate += (Tasks[i].WCET/Tasks[i].period)*100;
  }

  if (totalrate > 100){
    printf("Le taux d'utilisation du CPU ne peut etre superieur a 100 pourcents\n");
  }

  pthread_t Threads[TaskNbr];

  /* Initialisation des vars. partagees */

  pthread_cond_init(&Shared_T.Cond_Var, NULL);
  pthread_mutex_init(&Shared_T.Lock, NULL);

  for (i = 0; i < TaskNbr; i++){
    arguments_Task.deadline = Tasks[i].deadline;
    printf("deadline = %f\n", arguments_Task.deadline);
    arguments_Task.number = Tasks[i].number;
    printf("tâche = %d\n", arguments_Task.number);
    pthread_create(&Threads[i], NULL, (void *)TaskExec, &arguments_Task);
    printf("vient d'etre cree : (0x)%x\n", (int) Threads[i]);
  }

  sleep(10);
	return 0;

}

void TaskExec(float d, int nbr){

  float w = Tasks[nbr].WCET;
  float q = 1000;
  int texec = 0;

  printf("Lancement fonction activate imminent\n");

  activate(nbr);

  printf("Fonction activate terminée\n");

  while(w > q){

    proceed(nbr);
    usleep(q);
    w -= q;

    texec += q;

  }

  complete(nbr);

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

  while(lTasks.first != &Tasks[nbr]){
    printf("Tâche %d en attente de passer prioritaire\n", nbr);
    pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
  }

  pthread_mutex_unlock(&Shared_T.Lock);

}

void complete(int nbr){

  pthread_cond_broadcast(&Shared_T.Cond_Var);

  Tasks[nbr].deadline += Tasks[nbr].period;

  Tasks[nbr].complete++;

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

  int i;

  /*if (*lTasks == NULL || *Tasks[nbr]  == NULL){
    exit(EXIT_FAILURE);
  }*/

  for (i = 0; i < MAX_TASKS; i++){

    if (Tasks[i].deadline > Tasks[nbr].deadline){

      if ((Tasks[i].previous)->deadline < Tasks[nbr].deadline && Tasks[i].previous != NULL){

        (Tasks[i].previous)->next = &Tasks[nbr];
        Tasks[nbr].previous = Tasks[i].previous;
        Tasks[i].previous = &Tasks[nbr];
        Tasks[nbr].next = &Tasks[i];

        printf("Tâche ajoutée entre %d et %d\n", (Tasks[nbr].previous)->number, (Tasks[nbr].next)->number);

      }

      else if (Tasks[i].previous == NULL){

        Tasks[nbr].previous = NULL;
        Tasks[nbr].next = &Tasks[i];
        Tasks[i].previous = &Tasks[nbr];
        lTasks->first = &Tasks[nbr];

        printf("Tâche %d ajoutée en première position\n", nbr);

      }

    }

  }

}
