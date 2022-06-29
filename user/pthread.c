#include "lib.h"
#include <error.h>
#include <mmu.h>

// create a thread
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void * (*start_rountine)(void *), void *arg) {
	int newthread = syscall_thread_alloc(); //get the number of the thread in the process
	if (newthread < 0) {
		*thread = 0;
		return newthread;
	}
	struct Tcb *t = &env->env_threads[newthread];
	t->tcb_tf.regs[29] = USTACKTOP - 4 * BY2PG * newthread; // set the stack
	t->tcb_tf.pc = start_rountine; // set the start
	t->tcb_tf.regs[29] -= 4; 
	t->tcb_tf.regs[4] = arg; //
	t->tcb_tf.regs[31] = exit_son; // set the exit function
	syscall_set_thread_status(t->thread_id, ENV_RUNNABLE);	
	*thread = t->thread_id;
	return 0;
}

// only exit the target thread
void pthread_exit(void *value_ptr) {
	u_int threadid = syscall_getthreadid(); //
	struct Tcb *t = &env->env_threads[threadid & 0xf];
	t->tcb_exit_ptr = value_ptr;
	syscall_thread_destroy(threadid);
}

// set the thread's cancel state
int pthread_setcancelstate(int state, int *oldvalue) {
	u_int threadid = syscall_getthreadid();
	struct Tcb *t = &env->env_threads[threadid & 0xf];
	// wrong state 
	if ((state != THREAD_CAN_BE_CANCELED) & (state != THREAD_CANNOT_BE_CANCELED)) {
		return -1;
	}

	if (t->thread_id != threadid) {
		return -1;
	}

	if (oldvalue != 0) {
		*oldvalue = t->tcb_cancelstate;
	}

	t->tcb_cancelstate = state;
	return 0;
}

// set the thread's cancel type
int pthread_setcanceltype(int type, int *oldvalue) {
	u_int threadid = syscall_getthreadid();
	struct Tcb *t = &env->env_threads[threadid & 0xf];
	// wrong type
	if ((type != THREAD_CANCEL_IMI) & (type != THREAD_CANCEL_POINT)) {
		return -1;
	}

	if (t->thread_id != threadid) {
		return -1;
	}

	if (oldvalue != 0) {
		*oldvalue = t->tcb_canceltype;
	}

	t->tcb_canceltype = type;
	return 0;
}

// set a cancel point
void pthread_testcancel() {
	u_int threadid = syscall_getthreadid();
	struct Tcb *t = &env->env_threads[threadid & 0xf];
	
	if (t->thread_id != threadid) {
		user_panic("panic at pthread_testcancel!\n");
	}

	if ((t->tcb_canceled) && (t->tcb_cancelstate == THREAD_CAN_BE_CANCELED) && (t->tcb_canceltype == THREAD_CANCEL_POINT)) {
		t->tcb_exit_value = -THREAD_CANCELED_EXIT;
		t->tcb_exit_ptr = &(t->tcb_exit_value);
		syscall_thread_destroy(t->thread_id);
	}
}

// cancel a thread
int pthread_cancel(pthread_t thread) {
	struct Tcb *t = &env->env_threads[thread & 0xf];
	if ((t->thread_id != thread) || (t->tcb_status == ENV_FREE)) {
		return -E_THREAD_NOTFOUND;
	}
	// can't be canceled
	if (t->tcb_cancelstate == THREAD_CANNOT_BE_CANCELED) {
		return -E_THREAD_CANNOTCANCEL;
	}

	t->tcb_exit_value = -THREAD_CANCELED_EXIT;
	t->tcb_exit_ptr = &(t->tcb_exit_value);

	writef("now the exit value is %d\n", *((int *)t->tcb_exit_ptr));
	// cancel immediately
	if (t->tcb_canceltype == THREAD_CANCEL_IMI) {
		writef("now excute this\n");
		syscall_thread_destroy(thread);
	} else {
		t->tcb_canceled = 1;
	}
	return 0;
}

// detach a thread
int pthread_detach(pthread_t thread) {
	struct Tcb *t = &env->env_threads[thread & 0xf];
	int r;
	int i;
	if (t->thread_id != thread) {
		return -E_THREAD_NOTFOUND;
	}

	if (t->tcb_status == ENV_FREE) {
		u_int sp = USTACKTOP - BY2PG * 4 * (thread & 0xf);
		for(i = 1; i <= 4; ++i) {
			r = syscall_mem_unmap(0, sp - i * BY2PG);
			if (r < 0)
				return r;
		}
		user_bzero(t, sizeof(struct Tcb));
	} else {
		t->tcb_detach = 1;
	}
	return 0;
}

// join the target thread
int pthread_join(pthread_t thread, void **value_ptr) {
	int r = syscall_thread_join(thread, value_ptr);
	return r;
}

