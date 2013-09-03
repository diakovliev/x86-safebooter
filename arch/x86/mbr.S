#include <config.h>

/*

NOTES:
	- Now supports only read using int 0x13 extensions

*/

//#define __DEBUG_MBR__

/* Loader parameters block size */
#define BPB_SIZE			0x3e

/* Message output */
#ifdef __DEBUG_MBR__
#define MSG(x)				movw $(x),%si; call print_string
#else
#define MSG(x)
#endif/*__DEBUG_MBR__*/

.file "mbr.S"

.text

.code16gcc

_start:
	jmp real_start
	nop	

	. = _start + 4

disk_address_packet:
				.byte 0x10, 0x00
blocks_count:	.byte 0x01, 0x00
buffer:			.long LOADER_DESCRIPTOR_ADDRESS
lba_address:	.quad LOADER_DESCRIPTOR_LBA

	. = _start + BPB_SIZE

real_start:
	cli
	xorw %ax,%ax
	movw %ax,%ds
	movw %ax,%ss
	movw %ax,%es
	movw $LOADER_STACK_ADDRESS,%sp
	sti

/* Load loader descriptor. One sector, place to LOADER_DESCRIPTOR_ADDRESS */
load_descriptor:
	MSG(string_descriptor)
	movb $0x42,%ah
	movw $(disk_address_packet),%si
	int $0x13
	jnc loader_descriptor_pre_loaded
	MSG(string_fail)
	jmp fail

loader_descriptor_pre_loaded:
/* UnXOR */
	xor %ax,%ax
	mov blocks_count,%ax
	mov $DISK_SECTOR_SIZE,%bx
	push %dx
	mul %bx
	pop %dx
	mov %ax,%bx
	mov $LOADER_DESCRIPTOR_ADDRESS,%ax
	call xor_descramble_memory
/**/

	movw $LOADER_DESCRIPTOR_ADDRESS,%bx
	movw 9(%bx),%ax /* loader_descriptor_sectors_count */
	movw %ax,blocks_count
	movb $0x42,%ah
	movw $(disk_address_packet),%si
	int $0x13
	jnc loader_descriptor_loaded
	MSG(string_fail)
	jmp fail

loader_descriptor_loaded:
	MSG(string_ok)

/* UnXOR */
	xor %ax,%ax
	mov blocks_count,%ax
	mov $DISK_SECTOR_SIZE,%bx
	push %dx
	mul %bx
	pop %dx
	mov %ax,%bx
	mov $LOADER_DESCRIPTOR_ADDRESS,%ax
	call xor_descramble_memory
/**/

/* Get data from loader descriptor */
	movw $LOADER_DESCRIPTOR_ADDRESS,%bx
	movw 5(%bx),%ax /* loader_sectors_count */
	movw %ax,blocks_count
	movw $LOADER_CODE_ADDRESS,%bx
	movw %bx,buffer
	movw $LOADER_CODE_LBA,%bx
	movw %bx,lba_address

/* Load loader code. */
load_code:
	MSG(string_code)
	movb $0x42,%ah
	movw $(disk_address_packet),%si
	int $0x13     
	jnc loader_code_loaded
	MSG(string_fail)
	jmp fail

loader_code_loaded:
/* UnXOR */
	xor %ax,%ax
	mov blocks_count,%ax
	mov $DISK_SECTOR_SIZE,%bx
	push %dx
	mul %bx
	pop %dx
	mov %ax,%bx
	mov $LOADER_CODE_ADDRESS,%ax
	call xor_descramble_memory
/**/

	MSG(string_ok)
	jmp loader_start

loader_start:
	MSG(string_jump)
    mov $LOADER_CODE_ADDRESS,%eax
    jmp *%eax

#ifdef __DEBUG_MBR__
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

#endif/*__DEBUG_MBR__*/

#include "xor_descrambler.S"

/* Data */
#ifdef __DEBUG_MBR__
string_descriptor:	.string "D"
string_code:		.string "C"
string_fail:		.string "F"
string_ok:			.string "O"
string_jump:		.string "J"
#endif/*__DEBUG_MBR__*/

loader_sectors_count:		.byte 0x00
