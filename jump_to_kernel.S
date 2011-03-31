#include "loader.gen.h"

.file "jump_to_kernel.S"

.code16gcc

.text

.global jump_to_kernel_asm
.type jump_to_kernel_asm,@function
jump_to_kernel_asm:
	cli
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

