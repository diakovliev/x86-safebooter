#include <config.h>
#include <common.asm>

.file "jump_to_kernel.S"

.text

/**************************************************************/
.global jump_to_kernel_asm
.type	jump_to_kernel_asm, @function
jump_to_kernel_asm:

/* -------------------------------------- */	
/* protected mode code */

/* kernel real mode segment address */
	xor %eax,%eax
	mov $KERNEL_REAL_CODE_ADDRESS,%eax
	shr $4,%eax
	add $0x20,%eax
	mov %eax,kernel_segment

/* calculate r_mode_entry address */
	xor %eax,%eax
	lea r_mode_entry,%ax
	mov %ax,r_mode_offset

	/* Change code segment */
	mov gdt_r_code_segment_addr,%eax
	mov %eax,new_cs_segment
	LONG_JMP_(new_cs_segment,0,new_cs_offset,change_cs_offset)

.code16gcc

change_cs_offset:

/* set 16 bit data segment */
	mov gdt_r_data_segment_addr,%ax
	mov %ax,%ss
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs

/* go to real mode */
	mov %cr0,%eax
	and $0xFE,%al
	mov %eax,%cr0

/* final jump to real mode */
	FAR_JMP(r_mode_segment,r_mode_offset)

r_mode_entry:

/* -------------------------------------- */	
/* real mode code */
	xor %ebx,%ebx
	mov $KERNEL_REAL_CODE_ADDRESS,%ebx
	shr $4,%ebx
	mov %ebx,%ss
	mov %ebx,%ds
	mov %ebx,%es
	mov %ebx,%fs
	mov %ebx,%gs

/* jumping to kernel */
	FAR_JMP(kernel_segment,kernel_offset)

