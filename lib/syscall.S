#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <stackframe.h>
#include <unistd.h>

/*** exercise 4.2 ***/

NESTED(handle_sys,TF_SIZE, sp)
    SAVE_ALL                            // Macro used to save trapframe
    CLI                                 // Clean Interrupt Mask
    nop
    .set at                             // Resume use of $at

    // TODO: Fetch EPC from Trapframe, calculate a proper value and store it back to trapframe.
	lw t0, TF_EPC(sp)
	addiu t0, t0, 4
	sw t0, TF_EPC(sp)	
    // TODO: Copy the syscall number into $a0.
	lw a0, TF_REG4(sp)
	
    addiu   a0, a0, -__SYSCALL_BASE     // a0 <- relative syscall number
    sll     t0, a0, 2                   // t0 <- relative syscall number times 4
    la      t1, sys_call_table          // t1 <- syscall table base
    addu    t1, t1, t0                  // t1 <- table entry of specific syscall
    lw      t2, 0(t1)                   // t2 <- function entry of specific syscall

    lw      t0, TF_REG29(sp)            // t0 <- user's stack pointer
    lw      t3, 16(t0)                  // t3 <- the 5th argument of msyscall
    lw      t4, 20(t0)                  // t4 <- the 6th argument of msyscall

    // TODO: Allocate a space of six arguments on current kernel stack and copy the six arguments to proper location
	lw a0, TF_REG4(sp)
	lw a1, TF_REG5(sp)
	lw a2, TF_REG6(sp)
	lw a3, TF_REG7(sp)
	addiu sp, sp, -24
	sw t4, 20(sp)
	sw t3, 16(sp)
	#sw a3, 12(sp)
	#sw a2, 8(sp)
	#sw a1, 4(sp)
	#sw a0, 0(sp)

    jalr    t2                          // Invoke sys_* function
    nop

    // TODO: Resume current kernel stack
	addiu sp, sp, 24
    sw      v0, TF_REG2(sp)             // Store return value of function sys_* (in $v0) into trapframe

    j       ret_from_exception          // Return from exeception
    nop
END(handle_sys)

/*
NESTED(handle_sys, TF_SIZE, sp)
    SAVE_ALL
    CLI
    nop
    .set at

    lw t0, TF_EPC(sp)
    addiu t0, t0, 4
    sw t0, TF_EPC(sp)

    lw a0, TF_REG4(sp)//sysno

    addiu a0, a0, -__SYSCALL_BASE
    sll t0, a0, 2
    la t1, sys_call_table
    addu t1, t1, t0
    lw t2, 0(t1)//the addr of func

    lw a1, TF_REG5(sp) // number of var
    lw s0, TF_REG29(sp) // user stack

    addiu t1, a1, 1
    sll s1, t1, 2 // offset
    subu sp, sp, s1

    addiu t0, s0, 20
    addiu t4, sp, 16

    li t1, 6
    addiu t3, a1, 2

loop:
	bgt t1, t3, endloop
    nop
    lw t5, 0(t0)
    sw t5, 0(t4)
    addiu t0, t0, 4
    addiu t4, t4, 4
    addiu t1, t1, 1
    j loop
    nop
endloop:
	addiu a0, a0, __SYSCALL_BASE
    move t0, a1 //get the number
    move a1, a2
    move a2, a3

    blt t0, 3, endAgv3
    nop
    lw a3, 16(s0)
endAgv3:
    jalr t2
    nop
    addu sp, sp, s1
    sw v0, TF_REG2(sp)
    j ret_from_exception
    nop
END(handle_sys)
*/
sys_call_table:                         // Syscall Table
    .align 2
    .word sys_putchar
    .word sys_getenvid
    .word sys_yield
    .word sys_env_destroy
    .word sys_set_pgfault_handler
    .word sys_mem_alloc
    .word sys_mem_map
    .word sys_mem_unmap
    .word sys_env_alloc
    .word sys_set_env_status
    .word sys_set_trapframe
    .word sys_panic
    .word sys_ipc_can_send
    .word sys_ipc_recv
    .word sys_cgetc
	.word sys_thread_destroy
    .word sys_getthreadid
    .word sys_thread_alloc
    .word sys_set_thread_status
    .word sys_thread_join
   // .word sys_sem_destroy
   // .word sys_sem_wait
   // .word sys_sem_trywait
   // .word sys_sem_post
   // .word sys_sem_getvalue
