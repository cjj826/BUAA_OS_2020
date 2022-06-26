#include <env.h>
#include <pmap.h>
#include <printf.h>

/*
void sched_yield(void)
{

    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
    static struct Env *e = NULL;
   // hint:
    // *  1. if (count==0), insert `e` into `env_sched_list[1-point]`
   //  *     using LIST_REMOVE and LIST_INSERT_TAIL.
   //  *  2. if (env_sched_list[point] is empty), point = 1 - point;
   //  *     then search through `env_sched_list[point]` for a runnable env `e`, 
   //  *     and set count = e->env_pri
   //  *  3. count--
    // *  4. env_run()
    // *
   // *  functions or macros below may be used (not all):
    // *  LIST_INSERT_TAIL, LIST_REMOVE, LIST_FIRST, LIST_EMPTY
 
        if(count == 0 || e == NULL || e->env_status != ENV_RUNNABLE){
	        if(e != NULL){//count == 0 || status is not runnable
    	        LIST_REMOVE(e, env_sched_link);
        	    if(e->env_status != ENV_FREE){
            	    LIST_INSERT_TAIL(&env_sched_list[1-point], e, env_sched_link);
            	}
        	}
       		 while(1){
            	while(LIST_EMPTY(&env_sched_list[point]))
                	point = 1 - point;
            	e = LIST_FIRST(&env_sched_list[point]);
            	if(e->env_status == ENV_FREE){
                	LIST_REMOVE(e, env_sched_link);
            	} else if(e->env_status == ENV_NOT_RUNNABLE){
                	LIST_REMOVE(e, env_sched_link);
                	LIST_INSERT_TAIL(&env_sched_list[1-point], e, env_sched_link);
            	} else {
                	count = e->env_pri;
                	break;
            	}
        	}
    	}
	
	count--;
	if (e != NULL) {
		env_run(e);
	}
    //env_run(LIST_FIRST(&env_sched_list[0]));
}*/

void sched_yield(void)
{

    static int count = 0; // remaining time slices of current env
    static int point = 0; // current env_sched_list index
    static struct Env *e = NULL;
    static struct Tcb *t = NULL;

        if(count == 0 || t == NULL || t->tcb_status != ENV_RUNNABLE){
	        if(t != NULL){//count == 0 || status is not runnable
    	        LIST_REMOVE(t, tcb_sched_link);
        	    if(t->tcb_status != ENV_FREE){
            	    LIST_INSERT_TAIL(&tcb_sched_list[1-point], t, tcb_sched_link);
            	}
        	}
       		 while(1){
            	while(LIST_EMPTY(&tcb_sched_list[point]))
                	point = 1 - point;
            	t = LIST_FIRST(&tcb_sched_list[point]);
            	if(t->tcb_status == ENV_FREE){
                	LIST_REMOVE(t, tcb_sched_link);
            	} else if(t->tcb_status == ENV_NOT_RUNNABLE){
                	LIST_REMOVE(t, tcb_sched_link);
                	LIST_INSERT_TAIL(&tcb_sched_list[1-point], t, tcb_sched_link);
            	} else {
                	count = t->tcb_pri;
                	break;
            	}
        	}
    	}

	count --;
	if (t != NULL) {
		env_run(t);
	}
    //env_run(LIST_FIRST(&env_sched_list[0]));
}
