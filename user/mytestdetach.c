#include "lib.h"

void start_rountine1(void *arg) {
    u_int i = 0;
    while (i < 10) {
        i++;
        writef("thread1 is %d\n", i);
    }
	writef("thread1 is end\n");
}

void start_rountine2(void *arg) {
    u_int i = -1;
    while (i > -10) {
        i--;
        writef("thread2 is %d\n", i);
    }
    writef("thread2 is end\n");
}

void umain() {
    pthread_t thread1;
    pthread_t thread2;
    pthread_create(&thread1, NULL, start_rountine1, NULL); 						pthread_create(&thread2, NULL, start_rountine2, NULL);
    pthread_detach(thread2);
    int ret = pthread_join(thread2, 0);
    writef("thread0: the ret is %d\n", ret);
    pthread_join(thread1, 0);
    pthread_exit(0);
}
