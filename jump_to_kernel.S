#include "loader.gen.h"

.file "jump_to_kernel.S"

.text

.global jump_to_kernel_asm
.type jump_to_kernel_asm,@function
jump_to_kernel_asm:
	cli
	xor %eax,%eax
	mov $KERNEL_REAL_CODE_ADDRESS,%esi
	xor %ebp,%ebp
	xor %edi,%edi
	xor %ebx,%ebx
.byte	0xEA
.int	KERNEL_CODE_ADDRESS
kernel_seg:
.int	0x10


