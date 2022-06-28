#include "lib.h"

int n = 5;
sem_t tag, odd, even, cnt;

int oddcnt = 0, evencnt = 0;

void getodd() {
    if (oddcnt) {
        oddcnt--;
    	writef("get odd\n");
    }
    else user_panic("no odd");
}

void countodd() {
    writef("odd is %d\n", oddcnt);
}

void geteven() {
    if (evencnt) {
        evencnt--;
    	writef("get even\n");
    }
    else user_panic("no even");
}

void counteven() {
    writef("even is %d\n", evencnt);
}

void odd_t() {
    while (1) {
        sem_wait(&odd);
        sem_wait(&tag);
        getodd();
        sem_post(&tag);
        sem_post(&cnt);
        countodd();
    }
}

void even_t() {
    while (1) {
        sem_wait(&even);
        sem_wait(&tag);
        geteven();
        sem_post(&tag);
        sem_post(&cnt);
        counteven();
    } 
}

int produce() {
    static int x = 0;
	x = (x + 1) % 10;
    writef("produce num %d\n", x);
	return x;
}

void put(int x) {
    if (oddcnt + evencnt + 1 > n) {
        user_panic("full!");
    }
    if (x & 1) oddcnt++;
    else evencnt++;
    writef("odd is %d, even is %d, cnt is %d\n", oddcnt, evencnt, oddcnt + evencnt);
}

void umain() {
    sem_init(&tag, 0, 1); 
    sem_init(&odd, 0, 0); 
    sem_init(&even, 0, 0); 
    sem_init(&cnt, 0, n); 
    pthread_t thread; 
    pthread_create(&thread, NULL, odd_t, NULL); 
    pthread_create(&thread, NULL, even_t, NULL); 
    while(1){ 
        sem_wait(&cnt); 
        sem_wait(&tag); 
        int x = produce(); 
        put(x); 
        sem_post(&tag); 
        if(x & 1) sem_post(&odd); 
        else sem_post(&even); 
    }
}
