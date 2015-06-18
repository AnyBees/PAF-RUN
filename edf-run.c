#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "structures.h"

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_T;

typedef struct {
	// sigjmp_buf context;
	bool active; /* true when job is released till its done executing */
			/* time in microsec */
	unsigned long WCET; //Worst Case Execution Time
	unsigned long period;
	unsigned long deadline;
	unsigned long phase;
	unsigned long long abs_deadline;
	unsigned long long next_release;
	void* stack;
} task;

SD Tasks[30];

timespec ts;
timespec nextperiod;
//timeval tp;

int main (int argc, char *argv[]){
  int i;
  int rate;
  int period;
  int totalrate;
  int nbr;
  int firstdeadline = 32767;
  int firstexec = NULL;
  int nextexec = NULL;
  int nextdeadline = 32767;
  int timeexec = 0;

  // int checktime(int )

  if (argc != argv[1]){
    printf("Usage : %s nombre-de-taches\n");
    exit(1);
  }

  TaskNbr = atoi(argv[1]);

  /* Creation et initialisation du tableau contenant les periodes et deadlines des taches */

  // int TaskTab[2][TaskNbr];

  printf("Quels sont le taux d'utilisation et la periode pour chaque tache ?\n")

  for (i = 0, i < TaskNbr, i++){
    nbr = scanf("%d %d", &period, &rate);
    if (nbr != 2){
      printf();
    }

    Tasks.periods[i] = period;
    Tasks.rate[i] = rate;
  }

  for i = 0, i < TaskNbr, i++){
    totalrate += TaskTab[1][i]*0.01;
  }

  if (rate > 1){
    printf("Le taux d'utilisation ne peut etre superieur a 100 pourcents\n");
  }

  pthread_t Threads[TaskNbr];

  /* Initialisation des vars. partagees */

  pthread_cond_init(&Shared_T.Cond_Var, NULL);
  pthread_mutex_init(&Shared_T.Lock, NULL);

  /* creation des threads  */

  for (i = 0 ; i < TaskNbr ; i++){
    pthread_create(&Threads[i], NULL, (void *) Un_Thread, NULL);
    printf("vient d'etre cree : (0x)%x\n", (int)Threads[i]);
  }

  /* attendre la fin des threads precedemment crees */
	/*for (i = 0 ; i < TaskNbr ; i++){
		pthread_join (Threads[i], NULL);
		printf("fin de tid %x\n", (int)Threads[i]);
	}*/

  // return 0;

  while (true){

    for (i = 0; i < TaskNbr; i++){
      if (Tasks.deadlines[i] < firstdeadline && !Tasks.complete[i]){
        firstdeadline = Tasks.deadlines[i];
        firstexec = i;
      }
    }

    i = 0;

    while(i != firstdeadline){
      if (Tasks.deadlines[i] < nextdeadline && !Tasks.complete[i]){
        nextdeadline = Tasks.deadlines[i];
        nextexec = i;
      }
    }

    if (timeexec < Tasks.periods[nextexec]){
      clock_gettime(CLOCK_REALTIME, &ts);

      if ((nextperiod.tv_sec - ts.tv_sec) < Tasks.periods[firstexec]*Tasks.rate[firstexec]*0.01){
        ts.tv_sec += (nextperiod.tv_sec - ts.tv_sec);
        Tasks.completion[firstexec] = ((int)(nextperiod.tv_sec - ts.tv_sec)*100 / Tasks.periods[firstexec]*Tasks.rate[firstexec]);
      } else {
        ts.tv_sec += Tasks.periods[firstexec]*Tasks.rate[firstexec]*0.01;
        Tasks.completion[firstexec] = 100;
        Tasks.complete[firstexec] = 1;
      }

      pthread_mutex_lock(&Shared_T.Lock);
      pthread_cond_timedwait(&Shared_T.Cond_Var, &Shared_T.Lock, &ts);
      pthread_mutex_unlock(&Shared_T.Lock);

      Tasks.deadlines[firstexec] += Tasks.periods[firstexec];
      timexec += Tasks.periods[firstexec]*Tasks.rate[firstexec]*0.01;

      /*if (Tasks.periods[nextexec] < Tasks.periods[firstexec]){
        nextperiod.tv_sec = ts.tv_sec + (Tasks.periods[nextexec] - timeexec);
      }*/
    }

    // if (timeexec)

    // Acquittes++;
    printf("fin du thread (0x)%x\n", (int)Zone_Part.Thread_Id);
    /* Pour eviter que deux threads se terminent
     * l'un  a la suite de l'autre
     * sans que main ait pu acquitter le premier
     */
    // pthread_mutex_unlock(&Verrou_Acq);
  }
  pthread_mutex_unlock(&Shared_T.Lock);
  /* printf("main (Tid (0x)%x) : FIN, et nombre de threads acquittes : %d, nombre de threads termines : %d\n",
         (int)Tid_main, Acquittes,Zone_Part.Threads_Finis); */
  return 0;

}

/*int checktime(timespec nextperiod, pthread_cond_t Lock){
  timespec tc;
  while(&Lock){
    clock_gettime(CLOCK_REALTIME, &tc);
    if(tc.tv_sec)
  }
}*/

/*-----------------------------------------------------------------
                            Un_Thread
  Fonction executee par les threads.
  -----------------------------------------------------------------*/
void Un_Thread(void){
  pthread_t mon_tid;
  void Fonc1(void);

  mon_tid = pthread_self();
  printf("Thread (0x)%x : DEBUT\n", (int)mon_tid);
  Fonc1();
  printf("Thread (0x)%x : FIN\n", (int)mon_tid);
  pthread_exit(NULL);
}

/*-----------------------------------------------------------------
  Fonc1
  -----------------------------------------------------------------*/
void Fonc1(void){
  int i, Nbre_Iter;
  Nbre_Iter = random()/10;
  i=0;
  while(i < Nbre_Iter) i++;
  return;
}
