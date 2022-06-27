#include "lib.h"
#include <mmu.h>
#include <env.h>

void
exit(void)
{
	//close_all();
	//syscall_env_destroy(0);
	struct Tcb *t = &env->env_threads[syscall_getthreadid() & 0xf];
	t->tcb_exit_value = -1;//exit
	t->tcb_exit_ptr = &(t->tcb_exit_value);
	syscall_thread_destroy(0);
}


struct Env *env;
struct Tcb *tcb;

void
libmain(int argc, char **argv)
{
	// set env to point at our env structure in envs[].
	env = 0;	// Your code here.
    tcb = 0;
	//writef("xxxxxxxxx %x  %x  xxxxxxxxx\n",argc,(int)argv);
	int envid;
	envid = syscall_getenvid();
	envid = ENVX(envid);
	env = &envs[envid];
    //new
    int tcbid;
    tcbid = syscall_getthreadid();
	tcbid = tcbid & 0xf;
	tcb = &env->env_threads[tcbid];

	// call user main routine
	umain(argc, argv);
	// exit gracefully
	exit();
	//syscall_env_destroy(0);
}
