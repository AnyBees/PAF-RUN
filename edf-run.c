#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "structures.h"

typedef struct {
  pthread_cond_t  Cond_Var;
  pthread_mutex_t Lock;
  pthread_t       Thread_Id;
} Shared_T;

SD Tasks[30];

int main (int argc, char *argv[]){
  int i;
  int rate;
  int period;
  int totalrate;
  int nbr;

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

    TaskTab[1][i] = period;
    TaskTab[2][i] = rate;
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

  return 0;

}

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
