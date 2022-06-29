#include "lib.h"

void *test1(void *args) {
	int oldvalue;
	pthread_setcancelstate(1, &oldvalue);
	writef("oldstate is %d, and thread1 change state\n", oldvalue);
	while (1) {
		writef("thread1 is going!\n");
		//syscall_yield();
	}
}

void *test2(void *args) {
	int oldvalue;
	pthread_setcancelstate(1, &oldvalue);
	writef("oldstate is %d, and thread2 change state\n", oldvalue);
    writef("thread2 can be immediately stopped!\n");
	while (1) {
		writef("thread2 is going!\n");
		//syscall_yield();
	}
}

void *test3(void *args) {
    int oldvalue;
	pthread_setcancelstate(1, &oldvalue);
	writef("oldstate is %d, and thread3 change state\n", oldvalue);
	pthread_setcanceltype(1, &oldvalue);
	writef("oldtype is %d, and thread3 change type\n", oldvalue);
    while (1) {
        writef("please stop me!\n");
        pthread_testcancel();
        writef("sorry the thread0 has no time\n");
        syscall_yield();
    }
}

void umain() {
	int a[3];
	a[0] = 1;
	a[1] = 2;
	a[2] = 3;
	pthread_t thread1;
    pthread_t thread2;
    pthread_t thread3;
    
	if (pthread_create(&thread1, NULL, test1, (void *)a)) {
		user_panic("create thread fail!\n");
	}
    int ret = pthread_cancel(thread1);
	
    writef("the first try to cancel thread1 is %d\n", ret);
   
    syscall_yield();

	int *pret = &ret;

	pthread_cancel(thread1);
   // writef("the second try to cancel thread1 is %d\n", ret);
    pthread_join(thread1, &pret);

    writef("the thread1's exit value is %d\n", *pret);

	if (pthread_create(&thread2, NULL, test2, (void *)a)) {
		user_panic("create thread fail 2\n");
	}
    syscall_yield();
	 pthread_cancel(thread2);
	
	pthread_join(thread2, &pret);

   	writef("the thread2's exit value is %d\n", *pret);

    if (pthread_create(&thread3, NULL, test3, (void *)a)) {
		user_panic("create thread fail 3\n");
	}
	syscall_yield();
	syscall_yield();
    syscall_yield();
	syscall_yield();
	pthread_cancel(thread3);
	writef("thread0 now wants to cancel son!\n");
    syscall_yield();
	pthread_join(thread3, &pret);
	writef("the thread3's exit value is %d\n", *pret);
	pthread_exit(0);
}
