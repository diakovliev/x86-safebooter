#include "loader.h"

.file "loader_start.S"

.text

.code16

_start:
	cli
	xorw %ax,%ax
	movw %ax,%ss
	movw %ax,%es
	movw %ax,%ds
	movw $STACK_ADDRESS,%sp
	movw $STACK_ADDRESS,%bp
	sti

	calll C_start
	jmp fail

fail:
	cli
	hlt

