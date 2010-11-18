   1              		.file	"tools.c"
   2              	#APP
   3              		.code16
   4              	#NO_APP
   5              		.text
   6              		.p2align 4,,15
   7              	.globl lba2chs
   8              		.type	lba2chs, @function
   9              	lba2chs:
  10 0000 6655     		pushl	%ebp
  11 0002 66BA1104 		movl	$272696337, %edx
  11      4110
  12 0008 6689E5   		movl	%esp, %ebp
  13 000b 67668B4D 		movl	8(%ebp), %ecx
  13      08
  14 0010 6653     		pushl	%ebx
  15 0012 6689C8   		movl	%ecx, %eax
  16 0015 66C1E804 		shrl	$4, %eax
  17 0019 66F7E2   		mull	%edx
  18 001c 67668B45 		movl	12(%ebp), %eax
  18      0C
  19 0021 66C1EA02 		shrl	$2, %edx
  20 0025 67668910 		movl	%edx, (%eax)
  21 0029 6689C8   		movl	%ecx, %eax
  22 002c 66BA0541 		movl	$68174085, %edx
  22      1004
  23 0032 66F7E2   		mull	%edx
  24 0035 6689C8   		movl	%ecx, %eax
  25 0038 6629D0   		subl	%edx, %eax
  26 003b 66D1E8   		shrl	%eax
  27 003e 6601C2   		addl	%eax, %edx
  28 0041 67668B45 		movl	16(%ebp), %eax
  28      10
  29 0046 66C1EA05 		shrl	$5, %edx
  30 004a 6689D3   		movl	%edx, %ebx
  31 004d 6683E30F 		andl	$15, %ebx
  32 0051 67668918 		movl	%ebx, (%eax)
  33 0055 6689D0   		movl	%edx, %eax
  34 0058 66C1E006 		sall	$6, %eax
  35 005c 6629D0   		subl	%edx, %eax
  36 005f 67668B55 		movl	20(%ebp), %edx
  36      14
  37 0064 6629C1   		subl	%eax, %ecx
  38 0067 6683C101 		addl	$1, %ecx
  39 006b 6766890A 		movl	%ecx, (%edx)
  40 006f 665B     		popl	%ebx
  41 0071 665D     		popl	%ebp
  42 0073 C3       		ret
  43              		.size	lba2chs, .-lba2chs
  44              		.ident	"GCC: (Ubuntu 4.4.3-4ubuntu5) 4.4.3"
  45              		.section	.note.GNU-stack,"",@progbits
