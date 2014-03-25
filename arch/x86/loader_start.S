#include <config.h>
#include <common.asm>

/* Message output */
#define MSG(x)				movw $(x),%si; call print_string

.file "loader_start.S"

.text

.code16gcc

_start:
	jmp entry_point

/*************************************/
print_char:
	movb $0x0E,%ah
	movb $0x00,%bh
	movb $0x07,%bl
	int $0x10
	ret
print_string:
	mov (%si),%al
	inc %si
	or %al,%al
	jz print_screen_exit_function
	call print_char
	jmp print_string
print_screen_exit_function:
	ret

S0: .string "S0"
S1: .string "S1"
S2: .string "S2"

/*************************************/
entry_point:
    MSG(S0)

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

    MSG(S1)

	# Open A20 address line
	inb $0x92,%al
	orb $2,%al
	outb %al,$0x92

	# Init gdt
	call init_gdt

    MSG(S2)

/* set gdt */
	cli
	lgdt gdtr

/* go to protected mode */
	mov %cr0,%eax
	or $1,%al
	mov %eax,%cr0

	mov gdt_code_segment_addr,%eax
	mov %eax,pm_segment

	xor %eax,%eax

	/* jmpl _32entry */
	FAR_JMP_(pm_segment,0,pm_offset,_32entry)

.code32
_32entry:
	/* set selectors */
	mov gdt_data_segment_addr,%ax
	mov %ax,%ds
	mov %ax,%ss
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

	/* set stack address */
	mov $LOADER_STACK_ADDRESS,%esp

	# Call C 32-bit entry point
	pushl $LOADER_CODE_ADDRESS
	pushl $LOADER_DESCRIPTOR_ADDRESS
	call C_start

	cli
	hlt

.global _start
