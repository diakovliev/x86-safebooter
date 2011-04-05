#include "loader.h"

/*

NOTES:
	- Now supports only read using int 0x13 extensions

*/

/* Loader parameters block size */
#define BPB_SIZE			0x3e

/* Message output */
#define MSG(x)				movw $(x),%si; call print_string

/* Message output routes */
#define PRINT_IMPL	\
print_char:;	\
	movb $0x0E,%ah;	\
	movb $0x00,%bh;	\
	movb $0x07,%bl;	\
	int $0x10;	\
	ret;	\
print_string:;	\
	mov (%si),%al; \
	inc %si;	\
	or %al,%al; \
	jz print_screen_exit_function; \
	call print_char; \
	jmp print_string; \
print_screen_exit_function:; \
	ret

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
	MSG(string_ok)
	jmp loader_start

loader_start:
	MSG(string_jump)
    movw $LOADER_CODE_ADDRESS,%ax
    jmpw *%ax
//	.byte 0xEA
//	.word LOADER_CODE_ADDRESS
//	.word 0

PRINT_IMPL

fail:
	cli
	hlt

/* Data */
string_descriptor:	.string "DESCRIPTOR:" 
string_code:		.string "CODE:" 
string_fail:		.string "FAIL" 
string_ok:			.string "OK\n\r"
string_jump:		.string "JUMP TO CODE...\n\r"

loader_sectors_count:	.byte 0x00

/* Partitions table */
	. = _start + 440
disk_signature: 			.byte 0,0,0,0
reserved: 					.byte 0,0

/* Using GUID partitoinal table for me. So I need one entry */
/* Partition 1 */
partition1_status:			.byte 0x80		/* Active */
partition1_chs_start:		.byte 0x00,0x01,0x01		
											/* Not used by me */
partition1_type:			.byte 0xEE		/* EFI partition */
partition1_chs_end:			.byte 0x00,0x01,0x01		
											/* Not used by me */
partition1_lba_start:		.byte 0x3E,0x00,0x00,0x00	
											/* Start LBA address */
partition1_lba_sector_cnt:	.byte 0x2A,0x50,0x00,0x00	
											/* !!! Shall be filled during installation !!! */
	. = _start + 510
/* MBR signature */
.byte 0x55,0xAA

