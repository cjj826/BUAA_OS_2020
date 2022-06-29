#include "lib.h"

void *test1(void *arg) {
	int ret = -24;
	pthread_exit(&ret);
}

void *test2(void *arg) {
    writef("I can make it through the rain!\n");
    writef("thread2 is end!\n");
    return;
}

void *test3(void *arg) {
    while (1) {
        writef("No one can stop me!!!\n");  
    }
}
void umain() {
	pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
	int args[3];
	args[0] = 1;
	args[1] = 2;
	args[2] = 3;
	pthread_create(&thread1, NULL, test1, (void *)args);
   	
	int ret1 = 0;
    int* pret1 = &ret1;
    pthread_join(thread1, &pret1);
	
    writef("the thread1's ret1 is %d\n", *pret1);
    
    pthread_create(&thread2, NULL, test2, (void *)args);
    
    pthread_create(&thread3, NULL, test3, (void *)args);
    
	int ret2 = 0;
	int *pret2 = &ret2;

	pthread_join(thread2, &pret2);

    return;
}
