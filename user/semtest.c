#include "lib.h"

void *test1(void *args) {
	u_int father_thread = ((u_int *)args)[0];
	sem_t *sem = (sem_t *)((u_int *)args)[1];
	int a;
	int r;
	for (a = 0; a < 2; ++a) {
		r = sem_wait(sem);
		if (r < 0) {
			user_panic("fail P at son1: %d\n",r);
		}
		writef("son1 P! now a is %d\n",a);
	}
}

void *test2(void *args) {
	u_int father_thread = ((u_int *)args)[0];
	sem_t *sem = (sem_t *)((u_int *)args)[1];
	int b;
	int r;
	for (b = 0; b < 3; ++b) {
		r = sem_wait(sem);
		if (r < 0) {
			user_panic("fail P at son2: %d\n",r);
		}
		writef("son2 P! now b is %d\n",b);
	}
}

void *test3(void *args) {
	u_int father_thread = ((u_int *)args)[0];
	sem_t *sem = (sem_t *)((u_int *)args)[1];
	int c;
	int r;
	for(c = 0; c < 5; ++c) {
		r = sem_wait(sem);
		if (r < 0) {
			user_panic("fail P at son3: %d\n",r);
		}
		writef("son3 P! now c is %d\n",c);
	}	
}

void umain() {
	u_int a[2];
	a[0] = syscall_getthreadid();
	pthread_t thread1;
	pthread_t thread2;
	pthread_t thread3;
	sem_t mysem;
	sem_init(&mysem,0,1);
	int r;
	int value;
	r = sem_getvalue(&mysem, &value);
    writef("value is %d\n", value);
	a[1] = &mysem;
	
	pthread_create(&thread1, NULL, test1, (void *)a);
	pthread_create(&thread2, NULL, test2, (void *)a);
	pthread_create(&thread3, NULL, test3, (void *)a);
	//int value;
	r = sem_getvalue(&mysem, &value);
	writef("r is %d\n", r);
	if (r < 0) {
		user_panic("get value error: %d\n",r);
	}
	int i = 0;
	for (i = 0; i < 9; ++i) {
		if (value <= 0) {
			writef("father post!\n");
			sem_post(&mysem);
		} 
		syscall_yield();
		r = sem_getvalue(&mysem,&value);
		writef("value is %d\n", value);
		if (r < 0) {
			user_panic("r is %d\n",r);
		}
	}
	pthread_exit(0);
}
