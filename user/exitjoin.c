#include "lib.h"

void start_rountine(void *arg) {
    pthread_exit(2424242);
}

void umain() {
    pthread_t thread;
    pthread_create(&thread, NULL, start_rountine, NULL);
    int x = 0;
    pthread_join(thread, &x);
    writef("x is %d\n", x);
}
