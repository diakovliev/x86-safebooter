.file "gdt_table.S"
.code16gcc

.text 

__gdt_table:
		.space 8,0
		.space 8,0

/**************************************************************/

// Code descriptor 
//	base: 0x00000000 
//	limit: 0xFFFFF 
//	granumarity: 1
//
//              LIMI   CODE
gdt_code_segment:	
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
gdt_data_segment:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10010010,0b11001111,0x00

// Real mode code descriptor 
//	base: 0x00000000 
//	limit: 0xFFFF 
//	granumarity: 0
//
//              LIMI   CODE
gdt_r_code_segment:	
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
gdt_r_data_segment:	
		.word 0xFFFF,0x0000
//           P  STYPE   GD0 LIMI
//                EWA    B
.byte 0x00,0b10010010,0b00001111,0x00

/**************************************************************/
gdtr:						.word	.-__gdt_table-1
gdt_addr:					.int 	0
gdt_code_segment_addr: 		.int 	gdt_code_segment - __gdt_table
gdt_data_segment_addr: 		.int 	gdt_data_segment - __gdt_table
gdt_r_code_segment_addr: 	.int	gdt_r_code_segment - __gdt_table
gdt_r_data_segment_addr: 	.int 	gdt_r_data_segment - __gdt_table

/**************************************************************/
/* Calculate physical address of GDT and place it to gdt_addr */

.type	init_gdt, @function
init_gdt:		
		xor %eax,%eax
		mov %ds,%ax
		shl $4,%eax

#define LOAD_EFFECTIVE_ADDRESS(ADDR,VAR) \
		lea VAR,%dx; \
		add %edx,%eax; \
		mov %eax,ADDR;

		LOAD_EFFECTIVE_ADDRESS(gdt_addr, __gdt_table)

		ret

/**************************************************************/
.global gdtr
.global gdt_addr
.global gdt_code_segment_addr
.global gdt_data_segment_addr
.global gdt_r_code_segment_addr
.global gdt_r_data_segment_addr
.global	init_gdt
