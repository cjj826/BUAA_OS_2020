#include "lib.h"

void *test(void *arg) {
	int ret = -2;
	writef("son is begin\n");
	writef("son threadid is 0x%b\n", syscall_getthreadid());
	writef("the address is %x\n", &ret);
	pthread_exit(&ret);
	writef("son not exit!\n");
}

void umain() {
	writef("Begin\n");
	writef("create pthread\n");
	pthread_t thread;
	int args[3];
	args[0] = 1;
	args[1] = 2;
	args[2] = 3;
	pthread_create(&thread, NULL, test, (void *)args);
	writef("son is create!\n");
	writef("thread id is 0x%b\n", thread);
	while ((int)env->env_threads[thread & 0x7].tcb_exit_ptr == 0) {
		//writef("void is %d\n",(int)env->env_threads[thread&0x7].tcb_exit_ptr);
		writef("thread id is 0x%b\n", thread);
	}
	writef("the address is %x\n", (int *)env->env_threads[thread & 0x7].tcb_exit_ptr);
	writef("son exit ret is %d\n", *((int *)env->env_threads[thread & 0x7].tcb_exit_ptr));
}
