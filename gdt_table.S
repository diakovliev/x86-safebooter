#include "loader.h"

.file "gdt_table.S"
.code16gcc

.text 

__gdt_table:
		.space 8,0

/************************************************************/

// Code descriptor 
//	base: 0x00000000 
//	limit: 0xFFFFF 
//	granumarity: 1
//
//              LIMI   CODE
__code:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                CRA    B
.byte 0x00,0b10011010,0b11001111,0x00

// Data descriptor 
//	base: 0x00000000 
//	limit: 0xFFFFF 
//	granumarity: 1
//
//              LIMI   CODE
__data:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10010010,0b11001111,0x00

// Stack descriptor 
//	base: 0x00000000 
//	limit: 0xFFFFF 
//	granumarity: 1
//
//              LIMI   CODE
__stack:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10010110,0b11001111,0x00

/************************************************************/

__gdt_table_size: .word .-__gdt_table

.global __gdt
.global __gdt_addr
__gdt:		.word	__gdt_table_size - 1
__gdt_addr:	.int	0
 
/* Calculate physical address of GDT and place it to __gdt_addr */
.global calculate_gdt_address
.type calculate_gdt_address, @function
calculate_gdt_address:
		xor %eax,%eax
		mov LOADER_CODE_ADDRESS,%ax
		shl $4,%eax
		add __gdt_table,%eax
		mov %eax,__gdt_addr
		ret

