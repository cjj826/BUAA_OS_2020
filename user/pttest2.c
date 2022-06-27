#include "lib.h"

int x = 0;

void start_rountine(void *arg){
	while(x < 50) writef("son:%d\n", ++x);
	writef("son end\n");
}

void umain() {
    pthread_t thread;
    pthread_create(&thread, NULL, start_rountine, NULL);
    while(x < 50) writef("father:%d\n", ++x);
    writef("father end\n");
}
