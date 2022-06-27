#include "lib.h"

void start_rountine1(void *arg) {
	writef("thread1 is end\n");
}

void start_rountine2(void *arg) {
    writef("thread2 is end\n");
}

void umain() {
    pthread_t thread; 
    pthread_create(&thread, NULL, start_rountine1, NULL); 							pthread_create(&thread, NULL, start_rountine2, NULL);
    pthread_detach(thread);
}
