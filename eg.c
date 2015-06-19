

/* edf_example.c */
/* an example program using tasks library */
#include "edf.h"
#include <stdio.h>

/* function execucted by task0 */
void task0()
{
	int i;
	/* infinite loop: each iteration is one job instance */
	while(1)
	{
		for ( i = 0; i < 5; ++ i )
		{	
			printf( "I'm task #0: %d\n", i );
		}
		/* job is done. Call scheduler */
		taskYield();
	}
}

void task1()
{
	int i;
	while(1)
	{
		for ( i = 0; i < 5; ++ i )
		{	
			printf( " I'm task #1: %d\n", i );
		}
		taskYield();
	}
}
void task2()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #2: %d\n", i );
                }
                taskYield();
        }
}
void task3()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #3: %d\n", i );
                }
                taskYield();
        }
}

void task4()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #4: %d\n", i );
                }
                taskYield();
        }
}
void task5()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #5: %d\n", i );
                }
                taskYield();
        }
}
void task6()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #6: %d\n", i );
                }
                taskYield();
        }
}
void task7()
{
        int i;
        while(1)
        {
                for ( i = 0; i < 5; ++ i )
                {
                        printf( " I'm task #7: %d\n", i );
                }
                taskYield();
        }
}

/* Idle tasks called when there are no active jobs in the system */
void idletask()
{
	while(1) {}
}

int main()
{
/* Initialize the task library */
initTasks();
/* Spawn Tasks */
/* WCET, period, deadline, phase= 0 */
spawnTask (&task0, 4000,800, 7,0);
spawnTask (&task1, 4000,750, 21,0);
spawnTask (&task2, 4000,1600,5,0);
spawnTask (&task3, 4000,1000,25,0);
spawnTask (&task4, 4000,600,16,0);
spawnTask (&task5, 4000,1200,10,0);
spawnTask (&task6, 3900,1100,27,0);
spawnTask (&task7, 4000,1600,20,0);

spawnTask (&idletask, 0,0,0,0);
/* start scheduling and running the tasks */
Start();
return 0;
}



