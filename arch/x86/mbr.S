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
	call xor_encrypt_memory
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
	call xor_encrypt_memory
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
	call xor_encrypt_memory
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

/* %ax - buffer */
/* %bx - buffer_size */
xor_encrypt_memory:
    mov    %eax, mbr_xor_buffer_ptr
    mov    %eax, mbr_xor_buffer_stored_ptr
    mov    %ebx, mbr_xor_buffer_size
	mov    $mbr_xor_key, %eax
	mov    %eax, mbr_xor_key_ptr
	mov    %eax, mbr_xor_key_stored_ptr
    jmp check_buffer_bounds

xor_loop:
 	mov   mbr_xor_buffer_ptr,%eax
 	xor   %ecx,%ecx
    movb  (%eax), %cl
 	mov   mbr_xor_key_ptr,%eax
 	xor   %ebx,%ebx
    movb  (%eax), %bl

	movb  %cl,mbr_xor_b_next
	xor  mbr_xor_b,%bl

    xor   %bl, %cl
 	mov   mbr_xor_buffer_ptr,%eax
    movb  %cl, (%eax)

	movb  mbr_xor_b_next,%cl
	movb  %cl,mbr_xor_b

    incl  mbr_xor_key_ptr

    mov   mbr_xor_key_size, %eax
    dec   %eax
    add   mbr_xor_key_stored_ptr, %eax
    mov   mbr_xor_key_ptr, %ebx
    cmp   %ebx, %eax
    jae inc_buffer_ptr

	mov   mbr_xor_key_stored_ptr, %eax
	mov   %eax, mbr_xor_key_ptr

 inc_buffer_ptr:
    incl  mbr_xor_buffer_ptr

 check_buffer_bounds:
    mov   mbr_xor_buffer_stored_ptr, %eax
    add   mbr_xor_buffer_size, %eax
    mov   mbr_xor_buffer_ptr, %ebx
    cmp   %ebx, %eax
	ja xor_loop
    ret

fail:
	cli
	hlt

/* Variables */
mbr_xor_key_ptr:			.int 0x00000000
mbr_xor_key_stored_ptr:		.int 0x00000000
mbr_xor_buffer_ptr:			.int 0x00000000
mbr_xor_buffer_stored_ptr: 	.int 0x00000000
mbr_xor_buffer_size:		.int 0x00000000

mbr_xor_b:					.byte 0x00
mbr_xor_b_next:				.byte 0x00

/* Data */
#ifdef __DEBUG_MBR__
string_descriptor:	.string "D"
string_code:		.string "C"
string_fail:		.string "F"
string_ok:			.string "O"
string_jump:		.string "J"
#endif/*__DEBUG_MBR__*/

loader_sectors_count:	.byte 0x00
mbr_xor_key: 			.incbin "../../mbr_xor_key"
mbr_xor_key_size: 		.int . - mbr_xor_key

/* Partitions table */
	. = _start + 440
disk_signature: 			.byte 0,0,0,0
reserved: 					.byte 0,0

/* Using GUID partitoinal table for me. So I need one entry */
/* Partition 1 */

#if 0
partition1_status:			.byte 0x80		/* Active */
partition1_chs_start:		.byte 0x00,0x01,0x01		
											/* Not used by me */
partition1_type:			.byte 0xEE		/* EFI partition */
partition1_chs_end:			.byte 0x00,0x01,0x01		
											/* Not used by me */
partition1_lba_start:		.byte 0x00,0xA0,0x00,0x00	
											/* Start LBA address */
partition1_lba_sector_cnt:	.byte 0x00,0x90,0x01,0x00	
											/* !!! Shall be filled during installation !!! */
#endif

. = _start + 510
/* MBR signature */
.byte 0x55,0xAA
. = _start + 512

.global _start
