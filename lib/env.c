/* Notes written by Qian Liu <qianlxc@outlook.com>
  If you find any bug, please contact with me.*/

#include <mmu.h>
#include <error.h>
#include <env.h>
#include <kerelf.h>
#include <sched.h>
#include <pmap.h>
#include <printf.h>

struct Env *envs = NULL;        // All environments
struct Env *curenv = NULL;            // the current env

static struct Env_list env_free_list;    // Free list
struct Env_list env_sched_list[2];      // Runnable list
 
extern Pde *boot_pgdir;
extern char *KERNEL_SP;

static u_int asid_bitmap[2] = {0}; // 64


/* Overview:
 *  This function is to allocate an unused ASID
 *
 * Pre-Condition:
 *  the number of running processes should be less than 64
 *
 * Post-Condition:
 *  return the allocated ASID on success
 *  panic when too many processes are running
 */
static u_int asid_alloc() {
    int i, index, inner;
    for (i = 0; i < 64; ++i) {
        index = i >> 5;
        inner = i & 31;
        if ((asid_bitmap[index] & (1 << inner)) == 0) {
            asid_bitmap[index] |= 1 << inner;
            return i;
        }
    }
    panic("too many processes!");
}

/* Overview:
 *  When a process is killed, free its ASID
 *
 * Post-Condition:
 *  ASID is free and can be allocated again later
 */
static void asid_free(u_int i) {
    int index, inner;
    index = i >> 5;
    inner = i & 31;
    asid_bitmap[index] &= ~(1 << inner);
}

/* Overview:
 *  This function is to make a unique ID for every env
 *
 * Pre-Condition:
 *  e should be valid
 *
 * Post-Condition:
 *  return e's envid on success
 */
u_int mkenvid(struct Env *e) {
    u_int idx = e - envs;
    u_int asid = asid_alloc();
    return (asid << (1 + LOG2NENV)) | (1 << LOG2NENV) | idx;
}

/* Overview:
 *  Convert an envid to an env pointer.
 *  If envid is 0 , set *penv = curenv; otherwise set *penv = envs[ENVX(envid)];
 *
 * Pre-Condition:
 *  penv points to a valid struct Env *  pointer,
 *  envid is valid, i.e. for the result env which has this envid, 
 *  its status isn't ENV_FREE,
 *  checkperm is 0 or 1.
 *
 * Post-Condition:
 *  return 0 on success,and set *penv to the environment.
 *  return -E_BAD_ENV on error,and set *penv to NULL.
 */
/*** exercise 3.3 ***/
int envid2env(u_int envid, struct Env **penv, int checkperm)
{
    struct Env *e;
    /* Hint: If envid is zero, return curenv.*/
    /* Step 1: Assign value to e using envid. */
	if (envid == 0) {
		*penv = curenv;
		return 0;
	} else {
		e = &envs[ENVX(envid)];
	}

    if (e->env_status == ENV_FREE || e->env_id != envid) {
        *penv = NULL;
        return -E_BAD_ENV;
    }
    /* Hints:
     *  Check whether the calling env has sufficient permissions
     *    to manipulate the specified env.
     *  If checkperm is set, the specified env
     *    must be either curenv or an immediate child of curenv.
     *  If not, error! */
    /*  Step 2: Make a check according to checkperm. */
	if (checkperm) {
        if (e != curenv && e->env_parent_id != curenv->env_id) {
            *penv = NULL;
            return -E_BAD_ENV;
        }
    }
	
    *penv = e;
    return 0;
}

/* Overview:
 *  Mark all environments in 'envs' as free and insert them into the env_free_list.
 *  Insert in reverse order,so that the first call to env_alloc() returns envs[0].
 *
 * Hints:
 *  You may use these macro definitions below:
 *      LIST_INIT, LIST_INSERT_HEAD
 */
/***: exercise 3.2 ***/
void
env_init(void)
{
    int i;
    /* Step 1: Initialize env_free_list. */
	LIST_INIT(&env_free_list);
	LIST_INIT(env_sched_list);
	LIST_INIT(env_sched_list + 1);    
    for (i = NENV-1; i >= 0; i--) {
        envs[i].env_status = ENV_FREE;
        LIST_INSERT_HEAD(&env_free_list, envs + i, env_link);
    }	

    /* Step 2: Traverse the elements of 'envs' array,
     *   set their status as free and insert them into the env_free_list.
     * Choose the correct loop order to finish the insertion.
     * Make sure, after the insertion, the order of envs in the list
     *   should be the same as that in the envs array. */


}


/* Overview:
 *  Initialize the kernel virtual memory layout for 'e'.
 *  Allocate a page directory, set e->env_pgdir and e->env_cr3 accordingly,
 *    and initialize the kernel portion of the new env's address space.
 *  DO NOT map anything into the user portion of the env's virtual address space.
 */
/*** exercise 3.4 ***/
static int
env_setup_vm(struct Env *e)
{

    int i, r;
    struct Page *p = NULL;
    Pde *pgdir;

    /* Step 1: Allocate a page for the page directory
     *   using a function you completed in the lab2 and add its pp_ref.
     *   pgdir is the page directory of Env e, assign value for it. */
    if (r = page_alloc(&p)) {
        panic("env_setup_vm - page alloc error\n");
        return r;
    }
	p->pp_ref++;
	pgdir = (Pde *)page2kva(p);
    /* Step 2: Zero pgdir's field before UTOP. */
	for (i = 0; i < PDX(UTOP); i++) {
        pgdir[i] = 0;
    }

    /* Step 3: Copy kernel's boot_pgdir to pgdir. */
	for ( ; i < 1024; i++) {
        if (i != PDX(UVPT)) {
            pgdir[i] = boot_pgdir[i];
        }
    }

	for (i = PDX(ULIM); i < PTE2PT; i++) {
        pgdir[i] = 0;
    }
    /* Hint:
     *  The VA space of all envs is identical above UTOP
     *  (except at UVPT, which we've set below).
     *  See ./include/mmu.h for layout.
     *  Can you use boot_pgdir as a template?
     */
	e->env_pgdir = pgdir;
    e->env_cr3 = PADDR(pgdir);

    /* UVPT maps the env's own page table, with read-only permission.*/
    e->env_pgdir[PDX(UVPT)]  = e->env_cr3 | PTE_V;
    return 0;
}

/* Overview:
 *  Allocate and Initialize a new environment.
 *  On success, the new environment is stored in *new.
 *
 * Pre-Condition:
 *  If the new Env doesn't have parent, parent_id should be zero.
 *  env_init has been called before this function.
 *
 * Post-Condition:
 *  return 0 on success, and set appropriate values of the new Env.
 *  return -E_NO_FREE_ENV on error, if no free env.
 *
 * Hints:
 *  You may use these functions and macro definitions:
 *      LIST_FIRST,LIST_REMOVE, mkenvid (Not All)
 *  You should set some states of Env:
 *      id , status , the sp register, CPU status , parent_id
 *      (the value of PC should NOT be set in env_alloc)
 */
/*** exercise 3.5 ***/
int
env_alloc(struct Env **new, u_int parent_id)
{
    int r;
    struct Env *e;

    /* Step 1: Get a new Env from env_free_list*/
	if (LIST_EMPTY(&env_free_list)) {
    	*new = NULL;    
        return -E_NO_FREE_ENV;
    }

    /* Step 2: Call a certain function (has been completed just now) to init kernel memory layout for this new Env.
     *The function mainly maps the kernel address to this new Env address. */
	e = LIST_FIRST(&env_free_list);

    /* Step 3: Initialize every field of new Env with appropriate values.*/
	r = env_setup_vm(e);
	if (r) {
		*new = NULL;
        return -E_NO_FREE_ENV;
    }

	e->env_id = mkenvid(e);
    e->env_status = ENV_RUNNABLE;
    e->env_parent_id = parent_id;

    /* Step 4: Focus on initializing the sp register and cp0_status of env_tf field, located at this new Env. */
    e->env_tf.cp0_status = 0x1000100c;
	e->env_tf.regs[29] = USTACKTOP;

    /* Step 5: Remove the new Env from env_free_list. */
	LIST_REMOVE(e, env_link);
    *new = e;
	return 0;

}

/* Overview:
 *   This is a call back function for kernel's elf loader.
 * Elf loader extracts each segment of the given binary image.
 * Then the loader calls this function to map each segment
 * at correct virtual address.
 *
 *   `bin_size` is the size of `bin`. `sgsize` is the
 * segment size in memory.
 *
 * Pre-Condition:
 *   bin can't be NULL.
 *   Hint: va may be NOT aligned with 4KB.
 *
 * Post-Condition:
 *   return 0 on success, otherwise < 0.
 */
/*** exercise 3.6 ***/
/*static int load_icode_mapper(u_long va, u_int32_t sgsize,
                             u_char *bin, u_int32_t bin_size, void *user_data)
{
    struct Env *env = (struct Env *)user_data;
    struct Page *p = NULL;
    u_long i = 0;
    int r;
    u_long offset = va - ROUNDDOWN(va, BY2PG);
	long size = 0;

	if (bin == NULL) return -1;

	u_long perm = PTE_R;
//	
	if (offset) {
        size = BY2PG - offset;
        if (r = page_alloc(&p)) {
            return r;
        }
        //p->pp_ref++;
       	page_insert(env->env_pgdir, p, va - offset, perm);
        bcopy((void *)bin, (void *)(page2kva(p) + offset), MIN(bin_size, size));
    }
   
    for (i = size; i < bin_size; i += BY2PG) {
        
        if (r = page_alloc(&p)) {
            return r;
        }
        //p->pp_ref++;
        page_insert(env->env_pgdir, p, va + i, perm);
        bcopy((void *)(bin + i), (void *)(page2kva(p)), MIN(bin_size - i, BY2PG));
    }

    while (i < sgsize) {
        if (r = page_alloc(&p)) {
            return r;
        }
		//p->pp_ref++;
        page_insert(env->env_pgdir, p, va + i, perm);
        i += BY2PG;
    }
    
////
	if (offset) {
        size = MIN(bin_size, (BY2PG - offset));
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if (r = page_alloc(&p)) {
           		return r;
       	 	}
            page_insert(env->env_pgdir, p, va + i, perm);
        }
        bcopy((void *)bin, (void *)(page2kva(p) + offset), size);
        i += size;
    }
    
    for ( ; i < bin_size; i += size) {
        size = MIN(bin_size - i, BY2PG);
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if (r = page_alloc(&p)) {
           		return r;
       	 	}
            page_insert(env->env_pgdir, p, va + i, perm);
        }
        bcopy((void *)(bin + i), (void *)(page2kva(p)), size);
    }
    
	//	offset = i - ROUNDDOWN(i, BY2PG);
    offset = va + i - ROUNDDOWN((va + i), BY2PG);//
    if (offset) {
        size = MIN(BY2PG - offset, sgsize - i);
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if (r = page_alloc(&p)) {
           		return r;
       	 	}
            page_insert(env->env_pgdir, p, va + i, perm);
            //bcopy((void *)(bin + i - offset), (void *)(page2kva(p)), offset);
        } 
        bzero((void *)(page2kva(p) + offset), size);
     	i += size;   
    }
    
    while (i < sgsize) {
        size = MIN(BY2PG, sgsize - i);
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if (r = page_alloc(&p)) {
           		return r;
       	 	}
            page_insert(env->env_pgdir, p, va + i, perm);
        }
        bzero((void *)page2kva(p), size);
        i += size;
    }
///	
    return 0;
}*/
/*
static int load_icode_mapper(u_long va, u_int32_t sgsize,
                             u_char *bin, u_int32_t bin_size, void *user_data)
{
    struct Env *env = (struct Env *)user_data;
    struct Page *p = NULL;
    u_long i = 0;
    int r;
    u_long offset = va - ROUNDDOWN(va, BY2PG);
    long size = 0;

    if (bin == NULL) return -1;

    u_long perm = PTE_R;
    
    if (offset) {
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if (r = page_alloc(&p)) {
                return r;
            }
            if (r = page_insert(env->env_pgdir, p, va + i, perm)) {
                return r;
            }
        }
        size = MIN(bin_size, BY2PG - offset);
        bcopy((void *)bin, (void *)(page2kva(p) + offset), size);
    }

    for (i = size; i < bin_size; i += BY2PG) {

        if (r = page_alloc(&p)) {
            return r;
        }
        if (r = page_insert(env->env_pgdir, p, va + i, perm)) {
            return r;
        }
        size = MIN(bin_size - i, BY2PG);
        bcopy((void *)(bin + i), (void *)(page2kva(p)), size);
    }
    while (i < sgsize) {
        if (r = page_alloc(&p)) {
            return r;
        }
        if (r = page_insert(env->env_pgdir, p, va + i, perm)) {
            return r;
        }
        i += BY2PG;
    }
    return 0;
}*/
static int load_icode_mapper(u_long va, u_int32_t sgsize,
                             u_char *bin, u_int32_t bin_size, void *user_data)
{
    struct Env *env = (struct Env *)user_data;
    struct Page *p = NULL;
    u_long i = 0;
    int r;
    int size;
    u_long offset = va - ROUNDDOWN(va, BY2PG);

    if (offset > 0) {
        p = page_lookup(env->env_pgdir, va + i, NULL);
        if (p == 0) {
            if ((r = page_alloc(&p)) < 0) {
                return r;
            }
            if ((r = page_insert(env->env_pgdir, p, va + i, PTE_R)) < 0) {
                return r;
            }
        }
        size = MIN(BY2PG - offset, bin_size - i);
        bcopy((void*)bin, (void*)(page2kva(p) + offset), size);
        i += size;
    }

    /* Step 1: load all content of bin into memory. */
    for (; i < bin_size; i += BY2PG) {
        /* Hint: You should alloc a new page. */
        if ((r = page_alloc(&p)) < 0) {
            return r;
        }
        if ((r = page_insert(env->env_pgdir, p, va + i, PTE_R)) < 0) {
            return r;
        }
        size = MIN(BY2PG,bin_size - i);
        bcopy((void*)(bin + i), (void*)page2kva(p), size);
    }
    /* Step 2: alloc pages to reach `sgsize` when `bin_size` < `sgsize`.
     * hint: variable `i` has the value of `bin_size` now! */
    while (i < sgsize) {
        if ((r = page_alloc(&p)) < 0) {
            return r;
        }
        if ((r = page_insert(env->env_pgdir, p, va + i, PTE_R)) < 0) {
            return r;
        }
        bzero((void*)page2kva(p), BY2PG);
        i += BY2PG;
    }
    return 0;
}
/* Overview:
 *  Sets up the the initial stack and program binary for a user process.
 *  This function loads the complete binary image by using elf loader,
 *  into the environment's user memory. The entry point of the binary image
 *  is given by the elf loader. And this function maps one page for the
 *  program's initial stack at virtual address USTACKTOP - BY2PG.
 *
 * Hints:
 *  All mapping permissions are read/write including text segment.
 *  You may use these :
 *      page_alloc, page_insert, page2kva , e->env_pgdir and load_elf.
 */
/*** exercise 3.7 ***/
static void
load_icode(struct Env *e, u_char *binary, u_int size)
{
    /* Hint:
     *  You must figure out which permissions you'll need
     *  for the different mappings you create.
     *  Remember that the binary image is an a.out format image,
     *  which contains both text and data.
     */
    struct Page *p = NULL;
    u_long entry_point;
    u_long r;
    u_long perm;

    /* Step 1: alloc a page. */
	r = page_alloc(&p);
    if (r) return;

    /* Step 2: Use appropriate perm to set initial stack for new Env. */
    /* Hint: Should the user-stack be writable? */
	perm = PTE_R;
    r = page_insert(e->env_pgdir, p, USTACKTOP - BY2PG, perm);
    if (r) return;

    /* Step 3: load the binary using elf loader. */
	r = load_elf(binary, size, &entry_point, (void *)e, load_icode_mapper);
    if (r) return;

    /* Step 4: Set CPU's PC register as appropriate value. */
    e->env_tf.pc = entry_point;
}

/* Overview:
 *  Allocate a new env with env_alloc, load the named elf binary into
 *  it with load_icode and then set its priority value. This function is
 *  ONLY called during kernel initialization, before running the FIRST
 *  user_mode environment.
 *
 * Hints:
 *  this function wraps the env_alloc and load_icode function.
 */
/*** exercise 3.8 ***/
void
env_create_priority(u_char *binary, int size, int priority)
{
    struct Env *e;
    /* Step 1: Use env_alloc to alloc a new env. */
	env_alloc(&e, 0);
    /* Step 2: assign priority to the new env. */
	e->env_pri = priority;
	load_icode(e, binary, size);
    /* Step 3: Use load_icode() to load the named elf binary,
       and insert it into env_sched_list using LIST_INSERT_HEAD. */
	LIST_INSERT_HEAD(env_sched_list, e, env_sched_link);
}
/* Overview:
 * Allocate a new env with default priority value.
 *
 * Hints:
 *  this function calls the env_create_priority function.
 */
/*** exercise 3.8 ***/
void
env_create(u_char *binary, int size)
{
     /* Step 1: Use env_create_priority to alloc a new env with priority 1 */
	env_create_priority(binary, size, 1);
}

/* Overview:
 *  Free env e and all memory it uses.
 */
void
env_free(struct Env *e)
{
    Pte *pt;
    u_int pdeno, pteno, pa;

    /* Hint: Note the environment's demise.*/
    printf("[%08x] free env %08x\n", curenv ? curenv->env_id : 0, e->env_id);

    /* Hint: Flush all mapped pages in the user portion of the address space */
    for (pdeno = 0; pdeno < PDX(UTOP); pdeno++) {
        /* Hint: only look at mapped page tables. */
        if (!(e->env_pgdir[pdeno] & PTE_V)) {
            continue;
        }
        /* Hint: find the pa and va of the page table. */
        pa = PTE_ADDR(e->env_pgdir[pdeno]);
        pt = (Pte *)KADDR(pa);
        /* Hint: Unmap all PTEs in this page table. */
        for (pteno = 0; pteno <= PTX(~0); pteno++)
            if (pt[pteno] & PTE_V) {
                page_remove(e->env_pgdir, (pdeno << PDSHIFT) | (pteno << PGSHIFT));
            }
        /* Hint: free the page table itself. */
        e->env_pgdir[pdeno] = 0;
        page_decref(pa2page(pa));
    }
    /* Hint: free the page directory. */
    pa = e->env_cr3;
    e->env_pgdir = 0;
    e->env_cr3 = 0;
    /* Hint: free the ASID */
    asid_free(e->env_id >> (1 + LOG2NENV));
    page_decref(pa2page(pa));
    /* Hint: return the environment to the free list. */
    e->env_status = ENV_FREE;
    LIST_INSERT_HEAD(&env_free_list, e, env_link);
    LIST_REMOVE(e, env_sched_link);
}

/* Overview:
 *  Free env e, and schedule to run a new env if e is the current env.
 */
void
env_destroy(struct Env *e)
{
    /* Hint: free e. */
    env_free(e);

    /* Hint: schedule to run a new environment. */
    if (curenv == e) {
        curenv = NULL;
        /* Hint: Why this? */
        bcopy((void *)KERNEL_SP - sizeof(struct Trapframe),
              (void *)TIMESTACK - sizeof(struct Trapframe),
              sizeof(struct Trapframe));
        printf("i am killed ... \n");
        sched_yield();
    }
}

extern void env_pop_tf(struct Trapframe *tf, int id);
extern void lcontext(u_int contxt);

/* Overview:
 *  Restore the register values in the Trapframe with env_pop_tf, 
 *  and switch the context from 'curenv' to 'e'.
 *
 * Post-Condition:
 *  Set 'e' as the curenv running environment.
 *
 * Hints:
 *  You may use these functions:
 *      env_pop_tf , lcontext.
 */
/*** exercise 3.10 ***/

void
env_run(struct Env *e)
{
    /* Step 1: save register state of curenv. */
    /* Hint: if there is an environment running, 
     *   you should switch the context and save the registers. 
     *   You can imitate env_destroy() 's behaviors.*/
	/*struct Trapframe *old;
	old = (struct Trapframe *)(TIMESTACK - sizeof(struct Trapframe));
	if (curenv != NULL && curenv != e) {
    	//curenv -> env_tf = *old;
		bcopy((void*)old, (void*)(&(curenv->env_tf)), sizeof(struct Trapframe));
    	curenv -> env_tf.pc = curenv -> env_tf.cp0_epc;
	}*/
	if (curenv) {
        struct Trapframe *old;
        old = (struct Trapframe*)(TIMESTACK - sizeof(struct Trapframe));
        bcopy((void*)old, (void*)(&(curenv->env_tf)), sizeof(struct Trapframe));
        curenv->env_tf.pc = curenv->env_tf.cp0_epc;
    }

    /* Step 2: Set 'curenv' to the new environment. */
	curenv = e;

    /* Step 3: Use lcontext() to switch to its address space. */
	lcontext((int)e->env_pgdir);

    /* Step 4: Use env_pop_tf() to restore the environment's
     *   environment   registers and return to user mode.
     *
     * Hint: You should use GET_ENV_ASID there. Think why?
     *   (read <see mips run linux>, page 135-144)
     */
	env_pop_tf(&(e->env_tf), GET_ENV_ASID(e->env_id));
}

void env_check()
{
    struct Env *temp, *pe, *pe0, *pe1, *pe2;
    struct Env_list fl;
    int re = 0;
    /* should be able to allocate three envs */
    pe0 = 0;
    pe1 = 0;
    pe2 = 0;
    assert(env_alloc(&pe0, 0) == 0);
    assert(env_alloc(&pe1, 0) == 0);
    assert(env_alloc(&pe2, 0) == 0);

    assert(pe0);
    assert(pe1 && pe1 != pe0);
    assert(pe2 && pe2 != pe1 && pe2 != pe0);

    /* temporarily steal the rest of the free envs */
    fl = env_free_list;
    /* now this env_free list must be empty! */
    LIST_INIT(&env_free_list);

    /* should be no free memory */
    assert(env_alloc(&pe, 0) == -E_NO_FREE_ENV);

    /* recover env_free_list */
    env_free_list = fl;

    printf("pe0->env_id %d\n",pe0->env_id);
    printf("pe1->env_id %d\n",pe1->env_id);
    printf("pe2->env_id %d\n",pe2->env_id);

    assert(pe0->env_id == 1024);
    assert(pe1->env_id == 3073);
    assert(pe2->env_id == 5122);
    printf("env_init() work well!\n");

    /* check envid2env work well */
    pe2->env_status = ENV_FREE;
    re = envid2env(pe2->env_id, &pe, 0);

    assert(pe == 0 && re == -E_BAD_ENV);

    pe2->env_status = ENV_RUNNABLE;
    re = envid2env(pe2->env_id, &pe, 0);

    assert(pe->env_id == pe2->env_id &&re == 0);

    temp = curenv;
    curenv = pe0;
    re = envid2env(pe2->env_id, &pe, 1);
    assert(pe == 0 && re == -E_BAD_ENV);
    curenv = temp;
    printf("envid2env() work well!\n");

    /* check env_setup_vm() work well */
    printf("pe1->env_pgdir %x\n",pe1->env_pgdir);
    printf("pe1->env_cr3 %x\n",pe1->env_cr3);

    assert(pe2->env_pgdir[PDX(UTOP)] == boot_pgdir[PDX(UTOP)]);
    assert(pe2->env_pgdir[PDX(UTOP)-1] == 0);
    printf("env_setup_vm passed!\n");

    assert(pe2->env_tf.cp0_status == 0x10001004);
    printf("pe2`s sp register %x\n",pe2->env_tf.regs[29]);

    /* free all env allocated in this function */
    LIST_INSERT_HEAD(env_sched_list, pe0, env_sched_link);
    LIST_INSERT_HEAD(env_sched_list, pe1, env_sched_link);
    LIST_INSERT_HEAD(env_sched_list, pe2, env_sched_link);

    env_free(pe2);
    env_free(pe1);
    env_free(pe0);

    printf("env_check() succeeded!\n");
}

void load_icode_check() {
    /* check_icode.c from init/init.c */
    extern u_char binary_user_check_icode_start[];
    extern u_int binary_user_check_icode_size;
    env_create(binary_user_check_icode_start, binary_user_check_icode_size);
    struct Env* e;
    Pte* pte;
    u_int paddr;
    assert(envid2env(1024, &e, 0) == 0);
    /* text & data: 0x00401030 - 0x00409adc left closed and right open interval */
    assert(pgdir_walk(e->env_pgdir, 0x00401000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 0xc) == 0x8fa40000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x26300001);
    assert(pgdir_walk(e->env_pgdir, 0x00402000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x10800004);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00801821);
    assert(pgdir_walk(e->env_pgdir, 0x00403000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x80a20000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x24060604);
    assert(pgdir_walk(e->env_pgdir, 0x00404000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x04400043);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x00405000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x00406000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x00407000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x7f400000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x00408000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x0000fffe);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x00409000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 0x2aa) == 0x004099fc);
    printf("text & data segment load right!\n");
    /* bss        : 0x00409adc - 0x0040aab4 left closed and right open interval */
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 0x2b7) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);
    assert(pgdir_walk(e->env_pgdir, 0x0040a000, 0, &pte) == 0);
    assert(*((int *)KADDR(PTE_ADDR(*pte))) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 0x2ac) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 0x2ad) == 0x00000000);
    assert(*((int *)KADDR(PTE_ADDR(*pte)) + 1023) == 0x00000000);

    printf("bss segment load right!\n");

    env_free(e);
    printf("load_icode_check() succeeded!\n");
}

