#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "structures.h"
#include <stdbool.h>

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_Task;

typedef struct {
  int timec;
  int TaskN;
} arguments_T;

Shared_Task Shared_T;

arguments_T arguments_Task;

/*typedef struct {
	// sigjmp_buf context;
	_Bool active; // true when job is released till its done executing
	// time in microsec
	unsigned long WCET; // Worst Case Execution Time
	unsigned long period;
	unsigned long deadline;
	unsigned long phase;
	unsigned long long abs_deadline;
	unsigned long long next_release;
	void* stack;
} task;*/

struct TSK Tasks;

struct timespec ts;
struct timespec nextperiod;
//timeval tp;

pthread_cond_t cv[30];

int firstexec = -1;

void Un_Thread(int *i);

void Fonc1(int i);

void TaskExec(void *arg);

int main (int argc, char *argv[]){
  int i;
  int rate;
  int period;
  int totalrate = 0;
  int nbr;
  int firstdeadline = 32767;
  //int nextperiodtime;
  //int firstexec = (int) NULL;
  //int nextexec = (int) NULL;
  //int nextexec = -1;
  //int nextdeadline = 32767;
  //int timeis;
  int TaskNbr;
  //int w;
  //int timeexecute;

  if (argc != 2){
    printf("Usage : %s nombre-de-taches\n", argv[0]);
    exit(1);
  }

  TaskNbr = atoi(argv[1]);

  arguments_Task.TaskN = TaskNbr;

  /* Creation et initialisation du tableau contenant les periodes et deadlines des taches */

  printf("Quels sont le taux d'utilisation et la periode pour chaque tache ?\n");

  for (i = 0; i < TaskNbr; i++){
    nbr = scanf("%d %d", &period, &rate);
    if (nbr != 2){
      printf("L'usage est periode-tache taux-activite-tache\n");
    }

    Tasks.periods[i] = period;
    Tasks.rate[i] = rate;
	// pthread_cond_init(&cv[i], NULL);
  }

  for (i = 0; i < TaskNbr; i++){
    totalrate += Tasks.rate[i];
  }

  if (totalrate > 100){
    printf("Le taux d'utilisation ne peut etre superieur a 100 pourcents\n");
  }

  pthread_t Threads[TaskNbr];

  /* Initialisation des vars. partagees */

  pthread_cond_init(&Shared_T.Cond_Var, NULL);
  pthread_mutex_init(&Shared_T.Lock, NULL);

  for (i = 0; i < TaskNbr; i++){
    if (Tasks.deadlines[i] < firstdeadline && !Tasks.complete[i]){
      arguments_Task.timec = Tasks.periods[i]*Tasks.rate[i]*0.01;
  }

  /* creation des threads  */

  for (i = 0; i < TaskNbr; i++){
	//int j = i;
    pthread_create(&Threads[i], NULL, (void *)TaskExec, &arguments_Task);
    printf("vient d'etre cree : (0x)%x\n", (int) Threads[i]);
  }

  sleep(10);

}

  //TaskExec(timeexecute, TaskNbr);

  /*while (1){

    // clock_gettime(CLOCK_REALTIME, &ts); // Get the time
    // clock_gettime(CLOCK_REALTIME, &nextperiod); // Get the time

    // nextperiodtime = 32767;

    for (i = 0; i < TaskNbr; i++){*/
      /* Check which task has the first deadline
      *  Only if the task has not been completed for this period*/
      /*if (Tasks.deadlines[i] < firstdeadline) {
        // Get the next task start
        nextperiod.tv_sec += Tasks.deadlines[i];
      }

      if (Tasks.deadlines[i] < firstdeadline && !Tasks.complete[i]){
        firstdeadline = Tasks.deadlines[i]; // Get the first deadline
        firstexec = i; // Get the first task
      }
    }

    i = 0;*/

    /*while(i != firstdeadline){
      // Check which task has the next deadline
      //  Only if the task has not been completed for this period
      if (Tasks.deadlines[i] < nextdeadline && !Tasks.complete[i]){
        nextdeadline = Tasks.deadlines[i]; // Get the next deadline
        nextexec = i; // Get the next task
      }
    }*/
	// pthread_cond_signal(&cv[firstexec]);

    /* If the next task needs to be executed sooner than at the end
    *  of the first task execution time*/
    /*if ((nextperiod.tv_sec - ts.tv_sec) < Tasks.periods[firstexec]*Tasks.rate[firstexec]*0.01){
      ts.tv_sec += (nextperiod.tv_sec - ts.tv_sec); // Add part of the execution time
      // Add the completion rate for that very execution time
      Tasks.completion[firstexec] = ((int) (nextperiod.tv_sec - ts.tv_sec)*100 / Tasks.periods[firstexec]*Tasks.rate[firstexec]);
    } else {
      // Otherwise, execute the task for its whole execution time
      ts.tv_sec += Tasks.periods[firstexec]*Tasks.rate[firstexec]*0.01;
      Tasks.deadlines[firstexec] += Tasks.periods[firstexec]; // Period time added to the deadline
      Tasks.completion[firstexec] = 100; // Completion is at 100%
      Tasks.complete[firstexec] = 1; // The task doesn't need to be executed again until its next period
    }

    pthread_mutex_lock(&Shared_T.Lock); // Lock
    pthread_cond_timedwait(&Shared_T.Cond_Var, &Shared_T.Lock, &ts); // Wait for the execution of the thread
    pthread_mutex_unlock(&Shared_T.Lock); // Unlock

    printf("fin du thread (0x)%x\n", (int) Shared_T.Thread_Id);

  }*/

  return 0;

}

/*-----------------------------------------------------------------
                            Un_Thread
  Fonction executee par les threads.
  -----------------------------------------------------------------*/
void Un_Thread(int *i){
	int j = *i;
	pthread_t mon_tid;

	mon_tid = pthread_self();
	printf("Thread (0x)%x : DEBUT\n", (int) mon_tid);
	Fonc1(j);
	printf("Thread (0x)%x : FIN\n", (int) mon_tid);
	//pthread_exit(NULL);
}

/*-----------------------------------------------------------------
  Fonc1
  -----------------------------------------------------------------*/
void Fonc1(int i){
	pthread_mutex_lock(&Shared_T.Lock);
	while(firstexec != i){
		pthread_cond_wait(&Shared_T.Cond_Var, &Shared_T.Lock);
	}
	// Consomme CPU
	pthread_mutex_unlock(&Shared_T.Lock);
	return;
}

struct timeval s = {0,0};

int gettime(){ // Get the time
  struct timeval tv;
  int usec;

  if ((s.tv_sec == 0) && (s.tv_usec == 0)) return 0;

  gettimeofday(&tv, NULL);
  usec=(tv.tv_sec - s.tv_sec) * 1000000 + (tv.tv_usec - s.tv_usec);
  return usec;
}

void TaskExec(void *arg){

  //struct arguments args = arg;

  int q = 1000;
  int i = 0;
  int firstdeadline = 32767;
  int firstexec = 0;
  int timeis = 0;
  //int w = 0;
  //int timeexecute = 0;

  arguments_T *args = (arguments_T *) arg;

  int w = args->timec;
  int TaskNbrExec = args->TaskN;

  pthread_mutex_lock(&Shared_T.Lock);
  TaskNbrExec++;

  printf("La valeur de w = %d\n", w);
  printf("La valeur de q = %d\n", q);

  for (i= 0; i < TaskNbrExec; i++){
    if (Tasks.deadlines[i] < firstdeadline && !Tasks.complete[i]){
      firstdeadline = Tasks.deadlines[i];
      firstexec = i;
      printf("La tache prioritaire est %d\n", firstexec);
    }

  while (q <= w*1000){
    if (TaskNbrExec > 1){
      pthread_cond_wait(&Shared_T.Cond_Var,&Shared_T.Lock);
      for (i= 0; i < TaskNbrExec; i++){
        if (Tasks.deadlines[i] < firstdeadline && !Tasks.complete[i]){
          firstdeadline = Tasks.deadlines[i];
          firstexec = i;
          printf("La tache prioritaire est %d\n", firstexec);
        }
      }
    }

    pthread_mutex_unlock(&Shared_T.Lock);

    while (q <= w*1000){
      timeis = gettime();
      while (gettime() <= timeis + q){
        usleep(q/10);
        printf("J'ai attendu %d ms\n", q/10);
      }

      pthread_mutex_lock(&Shared_T.Lock);

      if (q <= w*1000 && Tasks.deadlines[firstexec] > firstdeadline){
          pthread_cond_broadcast(&Shared_T.Cond_Var);
      }

      pthread_mutex_unlock(&Shared_T.Lock);

      w = w - (gettime () - timeis);

      if (Tasks.deadlines[firstexec] > firstdeadline) break;
    }

    if (w*1000 < q) {
      pthread_mutex_lock (&Shared_T.Lock);

      Tasks.deadlines[firstexec] += Tasks.periods[firstexec];
      Tasks.complete[firstexec] = 1;

      TaskNbrExec--;
      pthread_cond_broadcast (&Shared_T.Cond_Var);

      printf("Je relache la Variable Conditionnelle\n");

      pthread_mutex_unlock (&Shared_T.Lock);
    }
  }
}
}
