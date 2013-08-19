#include <config.h>

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

	mov gdt_r_code_segment_addr,%eax
	mov %eax,_gdt_selector
	
	xor %eax,%eax

	/* jmpl to 16bit code segment */
	.byte 0xEA
	.int change_segment
_gdt_selector:
	.int gdt_r_code_segment_addr

.code16gcc
change_segment:

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


