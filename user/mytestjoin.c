#include "lib.h"
void *test1(void *args) {
    u_int thread0 = ((u_int *)args)[0];
	writef("thread1 is going\n");
	int ret = pthread_join(thread0, NULL);
    writef("thread1: the ret is %d\n", ret);
    writef("thread1 end\n");
}

void *test2(void *args) {
    u_int thread0 = ((u_int *)args)[0];
    writef("thread2 is going\n");
	int ret = pthread_join(thread0, NULL);
    writef("thread2: the ret is %d\n", ret);
    writef("thread2 end\n");
}

void umain() {
	u_int a[1];
    
	pthread_t thread1;
	pthread_t thread2;
	a[0] = syscall_getthreadid();
    
	pthread_create(&thread1, NULL, test1, (void *)a);
	pthread_create(&thread2, NULL, test2, (void *)a);
    
	u_int i = 0;
    
    while (i <= 10) {
        writef("thread0 %d\n", i);
        i++;
		syscall_yield();
    }
    
	pthread_exit(0);
}
