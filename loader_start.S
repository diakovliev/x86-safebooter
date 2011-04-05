#include "loader.h"

.file "loader_start.S"

.text

.code16gcc

_start:
	jmp entry_point

entry_point:
	cli
	xorw %ax,%ax
	movw %ax,%ss
	movw %ax,%es
	movw %ax,%ds
	movw $LOADER_STACK_ADDRESS,%sp
	movw $LOADER_STACK_ADDRESS,%bp
	sti

	# clear screen
	movb $3,%al
	int $0x10

	# Open A20 address line
	inb $0x92,%al
	orb $2,%al
	outb %al,$0x92

	# Call 16-bit entry point
	pushl $LOADER_CODE_ADDRESS
	pushl $LOADER_DESCRIPTOR_ADDRESS
	calll C_start
	jmp fail

fail:
	cli
	hlt

