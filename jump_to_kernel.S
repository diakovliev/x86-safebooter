#include "loader.gen.h"

.file "jump_to_kernel.S"

.code16gcc

.global jump_to_kernel_asm
.type jump_to_kernel_asm,@function
jump_to_kernel_asm:
	cli
	mov $KERNEL_REAL_CODE_ADDRESS,%ax
	mov %ax,%ss
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	mov %ax,%gs
.byte	0xEA
.short	0
.short	KERNEL_REAL_CODE_ADDRESS+0x20


