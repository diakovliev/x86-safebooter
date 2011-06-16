#include "gdt_table.gen.h"
#include "loader.gen.h"

.file "jump_to_kernel.S"

.text

/**************************************************************/
.global jump_to_kernel_asm
.type	jump_to_kernel_asm, @function
jump_to_kernel_asm:

/* -------------------------------------- */	
/* protected mode code */

/* calculate r_mode_entry address */
	xor %ax,%ax
	lea r_mode_entry,%ax
	mov %ax,r_mode_offset

	/* jmpl to 16bit code segment */
	.byte 0xEA
	.int change_segment
	.int GDT_R_CODE_SEGMENT

.code16gcc
change_segment:

/* set 16 bit data segment */
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
	.int 0

r_mode_entry:

/* -------------------------------------- */	
/* real mode code */

/* jumping to kernel */

	xor %eax,%eax
	mov $KERNEL_REAL_CODE_ADDRESS,%eax
	shr $4,%eax
	mov %eax,%ebx
	add $0x20,%eax
	mov %eax,kernel_seg
	mov %ebx,%ss
	mov %ebx,%ds
	mov %ebx,%es
	mov %ebx,%fs
	mov %ebx,%gs
.byte	0x66,0xEA
.int	0
kernel_seg:
.int	0


