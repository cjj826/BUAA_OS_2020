#include "lib.h"
void* sons(void *v){
	writef("son2_create\n");
	//exit(0);
}

void* thrd_start_routine(void* v)
{
    writef("son1_create\n");
    pthread_t thrdid;
    pthread_create(&thrdid, 0, &sons, 0);
	exit(0);
}
 
void umain()
{
        pthread_t thrdid;
        pthread_create(&thrdid, 0, &thrd_start_routine, 0);
        writef("main thread\n");       
		pthread_exit(0); 
        return;
}

