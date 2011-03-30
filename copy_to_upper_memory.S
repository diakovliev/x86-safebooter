#include "gdt_table.gen.h"

.file "copy_to_upper_memory.S"
.code16gcc

.text

p_mode_dst:		.int 0
p_mode_src:		.int 0
p_mode_size:	.int 0
.global p_mode_dst
.global p_mode_src
.global p_mode_size

/* real mode registers */
r_mode_sp: .int 0
r_mode_ss: .int 0
r_mode_ds: .int 0
r_mode_es: .int 0
r_mode_fs: .int 0
r_mode_gs: .int 0

/* stored ebp for access to function parameters */
r_mode_ebp: .int 0

/**************************************************************/
.global copy_to_upper_memory_asm
.type	copy_to_upper_memory_asm, @function
copy_to_upper_memory_asm:
	enter $0,$0

	mov 8(%ebp),%eax
	mov 12(%ebp),%edx
	mov 16(%ebp),%ecx

	mov %eax,p_mode_dst
	mov %edx,p_mode_src
	mov %ecx,p_mode_size

/* store registers */	
	mov %ss,r_mode_ss
	mov %ds,r_mode_ds
	mov %es,r_mode_es
	mov %fs,r_mode_fs
	mov %gs,r_mode_gs

/* real mode entry point */
	mov %ss,r_mode_segment
	lea r_mode_entry,%ax
	mov %ax,r_mode_offset

/* set gdt */
	cli
	lgdt gdtr

/* store stack pointer */
	mov %sp,r_mode_sp

/* go to protected mode */
	mov %cr0,%eax
	or $1,%al
	mov %eax,%cr0

	/* jmpl copy_to_upper_memory_asm_32_entry */
	.byte 0xEA
	.short copy_to_upper_memory_asm_32_entry
	.short GDT_CODE_SEGMENT
	
copy_to_upper_memory_asm_16_entry:

/* restore real mode */
	mov $GDT_R_DATA_SEGMENT,%ax
	mov %ax,%ss
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

/* go to real mode */
	mov %cr0,%eax
	and $0xFE,%al
	mov %eax,%cr0

/* final jump */
	.byte 0x66,0xEA
r_mode_offset: .int 0
r_mode_segment: .int 0

r_mode_entry:
	mov r_mode_ss,%ss
	mov r_mode_ds,%ds
	mov r_mode_es,%es
	mov r_mode_fs,%fs
	mov r_mode_gs,%gs
	mov r_mode_sp,%sp

/* restore interrapts */	
	sti

/* return */
	leave
	ret

.code32
copy_to_upper_memory_asm_32_entry:

/* set selectors */
	mov $GDT_DATA_SEGMENT,%ax
	mov %ax,%ds

	mov $GDT_DATA_SEGMENT,%ax
	mov %ax,%es

	mov $GDT_STACK_SEGMENT,%ax
	mov %ax,%ss

/* -------------------------------------- */	
/* copy data */
	mov p_mode_dst,%eax
	mov p_mode_src,%edx
	mov p_mode_size,%ecx

copy:
	xor %ebx,%ebx
	movb (%eax),%bl
	movb %bl,(%edx)

	inc %eax
	inc %edx
	loop copy

/* -------------------------------------- */	

	/* jmpl copy_to_upper_memory_asm_16_entry */
	.byte 0xEA
	.int copy_to_upper_memory_asm_16_entry
	.int GDT_R_CODE_SEGMENT

