#include <asm/regdef.h>
#include <asm/cp0regdef.h>
#include <asm/asm.h>
#include <stackframe.h>
#include <unistd.h>
LEAF(exit_son)
    move a0, v0
    j exit_son_
    nop
END(exit_son)
