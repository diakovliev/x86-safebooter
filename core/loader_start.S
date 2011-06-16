#include "loader.h"
#include "gdt_table.gen.h"

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

	# Init gdt
	call init_gdt

/* set gdt */
	cli
	lgdt gdtr

/* go to protected mode */
	mov %cr0,%eax
	or $1,%al
	mov %eax,%cr0

	/* jmpl _32_entry */
	.byte 0x66,0xEA
	.int _32entry
	.int GDT_CODE_SEGMENT

.code32
_32entry:
	/* set selectors */
	mov $GDT_DATA_SEGMENT,%ax
	mov %ax,%ds
	
#	mov $GDT_STACK_SEGMENT,%ax
#	mov %ax,%ss
	
	mov $GDT_DATA_SEGMENT,%ax
	mov %ax,%ss

	mov $GDT_DATA_SEGMENT,%ax
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

	/* set stack address */
	mov $LOADER_STACK_ADDRESS,%esp

#	sti

	# Call C 32-bit entry point
	pushl $LOADER_CODE_ADDRESS
	pushl $LOADER_DESCRIPTOR_ADDRESS
	call C_start

	cli
	hlt

