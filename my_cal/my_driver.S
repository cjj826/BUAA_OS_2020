#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>

LEAF(_my_getchar)
	li t0, 0xB0000000
	loop1:
		lb t1, 0(t0)
		beq t1, zero, loop1
		nop
	end_loop1:
	sb t1, 0(t0)
	
	li t2, 13
	bne t1, t2, end_loop2
	nop
	loop2:
		li t2, 10
		sb t2, 0(t0)
	end_loop2:

	or v0, zero, t1
	jr ra
END(_my_getchar) 

LEAF(_my_putchar)
	li t0, 0xB0000000
	sb a0, 0(t0)
	jr ra
END(_my_putchar)

LEAF(_my_exit)
	li t0, 0xB0000000
	sb zero, 0x10(t0)
	jr ra
END(_my_exit)
