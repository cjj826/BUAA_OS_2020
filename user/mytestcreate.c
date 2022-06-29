#include "lib.h"

int a;//global var

void *test(void *arg) {
    writef("thread run successful!\n");
    int arg1 = ((int *)arg)[0];
    int arg2 = ((int *)arg)[1];
    int arg3 = ((int *)arg)[2];
	int* arg4 = ((int *)arg)[3];
	
	writef("son: the address of c is %x\n", arg4);

    if (arg1 == 649 && arg2 == 686 && arg3 == 99) {
		writef("send parameter successful!\n");
	}

    
	writef("son: a is %d\n", a);
	a++;
	writef("son: a is %d\n", a);
	

	writef("share successful!\n");

   	int c = 24;
    writef("son: c is %d\n, the fathe' c is %d\n", c, *arg4);

	writef("son: stack is independent!\n");
    
	*arg4 = 888718;
	
	writef("son is end!\n");
}

void umain() {
    a = 0;
    int c = 100;
    ++a;
    int thread;
    int args[4];
    args[0] = 649;
    args[1] = 686;
    args[2] = 99;
	args[3] = &c; // the address of c in the thread stack
	writef("the address of c is %x\n", args[3]);
    pthread_t son;
    thread = pthread_create(&son, NULL, test, (void *) args);
	if (!thread) writef("create successful\n");

	writef("father: c is %d\n", c);
    c++;
	writef("father: c is %d\n", c);
	writef("father: stack independent\n");
    syscall_yield();

	writef("after son's change, father: c is %d\n", c);
    writef("end\n");
    pthread_exit(0);
}
