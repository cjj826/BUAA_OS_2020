/* See COPYRIGHT for copyright information. */

#ifndef _ENV_H_
#define _ENV_H_

#include "types.h"
#include "queue.h"
#include "trap.h"
#include "mmu.h" 

#define LOG2NENV	10
#define NENV		(1<<LOG2NENV)
#define ENVX(envid)	((envid) & (NENV - 1))
#define GET_ENV_ASID(envid) (((envid)>> 11)<<6)

#define THREAD_MAX 16// the max num of thread in the same env

//the cancel state of thread
#define THREAD_CAN_BE_CANCELED       1
#define THREAD_CANNOT_BE_CANCELED    0

//the cancel type of thread
#define THREAD_CANCEL_IMI            0
#define THREAD_CANCEL_POINT          1 // wait for next cancel point

#define THREAD_CANCELED_EXIT        24 // the exit value when cancel

// Values of env_status in struct Env
#define ENV_FREE	0
#define ENV_RUNNABLE		1
#define ENV_NOT_RUNNABLE	2

struct sem {
	u_int sem_envid;            //the env where the sem is in
   	u_int sem_head;             //the queue's head
    u_int sem_tail;             //the queue's tail
	char sem_name[16];          //name of the sem
	int sem_value;              // value of the sem
	int sem_shared;             // 0 shows the sem is only used in the sem_envid
	int sem_wait_count;         //the num of the process that is blocked 
	struct Tcb *sem_wait_queue[10];  //the queue
};


struct Tcb {
	//basic information
	struct Trapframe tcb_tf;            //the tf of thread 
	u_int thread_id;                    //the id of thread
	u_int tcb_status;                   //the status of thread
    u_int tcb_pri;                      //the pri of thread 

	//sched information
	LIST_ENTRY(Tcb) tcb_sched_link;     //the same as env_sched_link
   
	struct Tcb *tcb_joined;             //only one thread can join, otherwise will be undefined
    void **tcb_join_value_ptr;          //point to the join value
	u_int tcb_detach;                   //is the thread detached ?

	//exit information
	void *tcb_exit_ptr;                 //point to the exit value
    int tcb_exit_value;                 //the exit value

	//cancel information
	int tcb_cancelstate;                //cancel state
	int tcb_canceltype;                 //cancel type
	u_int tcb_canceled;                 //is the thread canceled ?

};

struct Env {
	LIST_ENTRY(Env) env_link;       // Free list
	u_int env_id;                   // Unique environment identifier
	u_int env_parent_id;            // env_id of this env's parent
	Pde  *env_pgdir;                // Kernel virtual address of page dir
	u_int env_cr3;
	LIST_ENTRY(Env) env_sched_link;
	
	// Lab 4 IPC
	u_int env_ipc_value;            // data value sent to us 
	u_int env_ipc_from;             // envid of the sender  
	u_int env_ipc_recving;          // env is blocked receiving
	u_int env_ipc_dstva;		    // va at which to map received page
	u_int env_ipc_perm;		        // perm of page mapping received

	u_int env_ipc_waiting_thread_no;// the waiting thread_no
	
	// Lab 4 fault handling
	u_int env_pgfault_handler;      // page fault state
	u_int env_xstacktop;            // top of exception stack

	// Lab 6 scheduler counts
	u_int env_runs;			// number of times been env_run'ed
	u_int env_thread_count;
	u_int env_nop[174];                  // align to avoid mul instruction

	// lab4 challenge
	struct Tcb env_threads[16];
};

LIST_HEAD(Env_list, Env);
LIST_HEAD(Tcb_list, Tcb);

extern struct Env *envs;		// All environments
extern struct Env *curenv;	        // the current env
extern struct Tcb *curtcb;      // the current thread  
extern struct Env_list env_sched_list[2]; // runnable env list
extern struct Tcb_list tcb_sched_list[2];//  runnable thread list

void env_init(void);
int env_alloc(struct Env **e, u_int parent_id);
int thread_alloc(struct Env *e, struct Tcb **t);

void env_free(struct Env *);
void env_create_priority(u_char *binary, int size, int priority);
void env_create(u_char *binary, int size);
void env_destroy(struct Env *e);

int envid2env(u_int envid, struct Env **penv, int checkperm);
int threadid2tcb(u_int threadid, struct Tcb **ptcb);

void env_run(struct Tcb *t); //now, the smallest unit of scheduling is thread


// for the grading script
#define ENV_CREATE2(x, y) \
{ \
	extern u_char x[], y[]; \
	env_create(x, (int)y); \
}
#define ENV_CREATE_PRIORITY(x, y) \
{\
        extern u_char binary_##x##_start[]; \
        extern u_int binary_##x##_size;\
        env_create_priority(binary_##x##_start, \
                (u_int)binary_##x##_size, y);\
}
#define ENV_CREATE(x) \
{ \
	extern u_char binary_##x##_start[];\
	extern u_int binary_##x##_size; \
	env_create(binary_##x##_start, \
		(u_int)binary_##x##_size); \
}

#endif // !_ENV_H_
