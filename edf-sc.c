#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "structures.h"
#include <stdbool.h>

#define MAX_TASKS = 256;

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task;

typedef struct {
  int deadline;
  int number;
} arguments_T;

Shared_Task Shared_T;

arguments_T arguments_Task;

struct TSK[MAX_TASKS] Tasks;

void TaskExec(double deadline);

int main (int argc, char *argv[]){

  int i;
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
    nbr = scanf("%lf %lf", &wcet, &period);
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
    arguments_Task.number = Tasks[i].number;
    pthread_create(&Threads[i], NULL, (void *)TaskExec, &arguments_Task);
    printf("vient d'etre cree : (0x)%x\n", (int) Threads[i]);
  }

  sleep(10);

}

void TaskExec(float d, int nbr){

  int n;
  int w = Tasks[nbr].WCET;
  int q = 1000;

  n = activate(d, nbr);

  while(w > q){

    continue(n);
    usleep(q);
    w -= q;

  }

  complete(n);

}

int activate(float d, int nbr){

  

}

void continue(int n){



}

void complete(int n){



}
