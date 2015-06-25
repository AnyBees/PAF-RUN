#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "structures.h"
#include <stdbool.h>

#define MAX_TASKS 8 // Maximum tasks number defined

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task; // Shared structure for threads

Shared_Task Shared_T;

TSK Tasks[MAX_TASKS];

void *TaskExec(void *i); // Function which is executed by each task
void complete(int nbr); // Function executed when a task has finished its execution for one period
void insertion(int nbr); // Function executed to add a task's deadline in the linked list

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

  TaskNbr = atoi(argv[1]); // Get the tasks number

  printf("Quels sont le WCET et la periode pour chaque tache ?\n");

  Tasks[0].number = 0; // Initialising the fake task's number
  Tasks[0].next = 0; // Same for its next pointer

  for (i = 1; i <= TaskNbr; i++){
    nbr = scanf("%f %f", &wcet, &period);
    if (nbr != 2){
      printf("L'usage est WCET-tache periode-tache\n");
      exit(1);
    }

    /* Define every component of the task structure (from structures.h)
    *  As seen before, the task 0 is used as a "fake" one*/

    Tasks[i].number = i;
    Tasks[i].period = period*1000000;
    Tasks[i].deadline = period*1000000;
    Tasks[i].WCET = wcet*1000000;
    Tasks[i].complete = 0;
		Tasks[i].next = 0;
  }

  for (i = 1; i <= TaskNbr; i++){
    totalrate += (Tasks[i].WCET/Tasks[i].period)*100; // Checking if the CPU isn't overcharged
  }

  if (totalrate > 100){
    printf("Le taux d'utilisation du CPU ne peut etre superieur a 100 pourcents\n");
  }

  pthread_t Threads[TaskNbr];

  // Initialising shared variables

  pthread_cond_init(&Shared_T.Cond_Var, NULL);
  pthread_mutex_init(&Shared_T.Lock, NULL);

  pthread_mutex_lock(&Shared_T.Lock); // Locking to create each thread without being interrupted

  for (i = 1; i <= TaskNbr; i++){
    int *arg = malloc(sizeof(*arg));
    *arg = i;
    pthread_create(&Threads[i], NULL, TaskExec, arg);
    printf("vient d'etre cree : (0x)%x\n", (int) Threads[i]);
  }

  sleep(1);

  pthread_mutex_unlock(&Shared_T.Lock); // Unlocking after "making sure" that every thread is created (with sleep())

	sleep(1);

	pthread_mutex_lock(&Shared_T.Lock); // Locking again, so that only one thread can access at the same time

  // If no task is active at some point, simulate the fact that the CPU is idle
	while(true){
		while(Tasks[0].next != 0){
			pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
		}

		usleep(1000000);
		printf("Processor is idle\n");

		texec += 1000000;
		pthread_mutex_unlock(&Shared_T.Lock); // Unlocking again
		usleep(1000);
	}

	return 0;

}

void *TaskExec(void *i){

  int nbr = *((int *) i); // Getting the task's number

  Tasks[nbr].active = 1; // Task is active

  float w = Tasks[nbr].WCET; // Execution time
	printf("w%d = %f\n", nbr, w);
  float q = 1000000; // Quantum

  insertion(nbr);

	while(true){

		while(w >= q && Tasks[nbr].active){ // If execution time is superior to the quantum and the task is active

			pthread_mutex_lock(&Shared_T.Lock);

			while(Tasks[0].next != nbr){ // Checking to see if the task is prioritary
				printf("Tâche %d en attente de passer prioritaire\n", nbr);
				pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock); // Waiting if not prioritary
			}

		  usleep(q); // Time spent sleeping, equivalent to CPU consumption by a thread/task
			printf("Tache %d a consomme un quantum\n", nbr);
		  w -= q; // Reducing the execution time by the sleeping time

		  texec += q; // Adding the time spent sleeping
			pthread_mutex_unlock(&Shared_T.Lock); // Unlocking again
			usleep(1000); // Synchronisation time (1ms)
		}

	  complete(nbr); // Completion of the task

		printf("Sleep %f\n",(Tasks[nbr].complete)*(Tasks[nbr].period)-texec);

		usleep((Tasks[nbr].complete)*(Tasks[nbr].period)-texec); // Thread is put to sleep until its next period start

	  pthread_mutex_lock(&Shared_T.Lock);
    insertion(nbr); // Reinsert the thread in the linked list, position corresponding to its deadline
    Tasks[nbr].active = 1; // Activating it again
		w = Tasks[nbr].WCET; // Reinitialising the execution time
		printf("Tache %d va se reactiver\n", nbr);

    printf("texec = %d\n", texec);
		pthread_mutex_unlock(&Shared_T.Lock);

  }

return 0;

}

void complete(int nbr){

  pthread_cond_broadcast(&Shared_T.Cond_Var); // Liberate the Conditional Variable for each thread

  /* Change the linked list order, as well as pushing the deadline by the period,
  *  changing the complete number (corresponding to the number of times a task has been completed),
  *  and deactivating the task */

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

	while(Tasks[i].next != 0){ // As long as the next task isn't the fake one
		if(Tasks[Tasks[i].next].deadline > Tasks[nbr].deadline){ // Check on the deadline, if inferior to checked task, insert before
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

	if (!inserted){ // If the deadline is superior to every task's one, insert at the end of the linked list

		Tasks[nbr].previous = i;
		Tasks[nbr].next = 0;
    Tasks[i].next = nbr;

		printf("Tâche %d ajoutée en dernière position, ie après %d\n", nbr, i);
	}

}
