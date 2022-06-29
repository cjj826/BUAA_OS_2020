#include <asm/asm.h>
#include <pmap.h>
#include <env.h>
#include <printf.h>
#include <kclock.h>
#include <trap.h>

void mips_init() {
	printf("init.c:\tmips_init() is called\n");
	printf("env is %d\n",sizeof(struct Env));
	printf("Tcb is %d\n",sizeof(struct Tcb));
	printf("Trapframe is %d\n",sizeof(struct Trapframe));
	
	mips_detect_memory();

	mips_vm_init();
	page_init();

	env_init();

	//ENV_CREATE(user_tltest);
	//ENV_CREATE(user_fktest);
	//ENV_CREATE(user_pingpong);
	//ENV_CREATE(user_pttest);
	//ENV_CREATE(user_ptexittest);
	//ENV_CREATE(user_ptcanceltest);
	//ENV_CREATE(user_ptjointest);
	//NV_CREATE(user_cjjtest);
	//ENV_CREATE(user_testsem);
	//ENV_CREATE(user_semtest);
	//ENV_CREATE(user_pttest2);
	//ENV_CREATE(user_exitjoin);
	//ENV_CREATE(user_detachtest);
	//ENV_CREATE(user_counttest);
	//ENV_CREATE(user_teste);
	
	//ENV_CREATE(user_mytestcreate);
	ENV_CREATE(user_mytestexit);
	//ENV_CREATE(user_mytestcancel);
	trap_init();
	kclock_init();


	while(1);
	panic("init.c:\tend of mips_init() reached!");
}

void bcopy(const void *src, void *dst, size_t len) {
	void *max = dst + len;

	// copy machine words while possible
	while (dst + 3 < max) {
		*(int *)dst = *(int *)src;
		dst+=4;
		src+=4;
	}

	// finish the remaining 0-3 bytes
	while (dst < max) {
		*(char *)dst = *(char *)src;
		dst+=1;
		src+=1;
	}
}

void bzero(void *b, size_t len) {
	void *max = b + len;

	// fill machine words while possible
	while (b + 3 < max) {
		*(int *)b = 0;
		b += 4;
	}

	// finish the remaining 0-3 bytes
	while (b < max) {
		*(char *)b++ = 0;
	}
}
