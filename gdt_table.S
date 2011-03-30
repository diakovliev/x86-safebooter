#include "loader.h"

.file "gdt_table.S"
.code16gcc

.text 

__gdt_table:
		.space 8,0

/**************************************************************/

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

// Real mode code descriptor 
//	base: 0x00000000 
//	limit: 0xFFFF 
//	granumarity: 0
//
//              LIMI   CODE
__r_code:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10011010,0b00001111,0x00

// Real mode data descriptor 
//	base: 0x00000000 
//	limit: 0xFFFF 
//	granumarity: 0
//
//              LIMI   CODE
__r_data:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10010010,0b00001111,0x00

/**************************************************************/
gdtr:		.word	.-__gdt_table-1
gdt_addr:	.int	0

/**************************************************************/
/* Calculate physical address of GDT and place it to gdt_addr */
.type	init_gdt, @function
init_gdt:		
		xor %eax,%eax
		mov %ds,%ax
		shl $4,%eax
		lea __gdt_table,%dx
		add %edx,%eax
		mov %eax,gdt_addr
		ret

/**************************************************************/
.global gdtr
.global gdt_addr
.global	init_gdt
