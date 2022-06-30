#include "lib.h"
int serving = 0;
sem_t mutex, emptyPot, fullPot;
int MAX = 10;

void putIn() {
    writef("put in!!!\n");
}

void cook() {
    while (1) {
        sem_wait(&emptyPot);
        putIn();
        sem_post(&fullPot);
    } 
}

void getFromPot() {
    writef("serving is %d, get from pot!!!\n", serving);
}

void savage() {
    while (1) {
        sem_wait(&mutex);
        if (serving == 0) {
            sem_post(&emptyPot);
            sem_wait(&fullPot);
            serving = MAX;
        }
        serving -= 1;
        getFromPot();
        sem_post(&mutex);
    }
}


void umain() {
    sem_init(&mutex, 0, 1);
    sem_init(&emptyPot, 0, 0);
    sem_init(&fullPot, 0, 0);
    
    pthread_t thread;
    pthread_create(&thread, NULL, cook, NULL); 
    pthread_create(&thread, NULL, savage, NULL);
    
    pthread_exit(0);    
}
