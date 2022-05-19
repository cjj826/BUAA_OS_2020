#include "lib.h"
/*
void umain() {
  */  /*writef(
        "Smashing some kernel codes...\n"
        "If your implementation is correct, you may see some TOO LOW here:\n"
    );
    *(int *) KERNBASE = 0;
    writef("My mission completed!\n");*/
	/*syscall_sum(1, 2, 3, 4, 5, 6);
	writef("come on!");
	//printf("\n%d\n", a);
}*/
void umain() {
    u_int me = syscall_getenvid();
    while (syscall_try_acquire_console() != 0) {
        syscall_yield();
    }
    writef("I'm %x\n", me);
    syscall_release_console();

    while(1);
}
