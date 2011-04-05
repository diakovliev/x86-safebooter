//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

asm(".code16gcc");

#include "loader.h"
#include "bios_tools.h"
#include "console_interface.h"
#include "string.h"
#include "lbp.h"
#include "copy_to_upper_memory.h"
#include "jump_to_kernel.h"

/* TODO: I have to find header with VK codes */
#define VK_ENTER 0x1C

/* TODO: Move to command processor header */
#define ERR_CMD_OK				0
#define ERR_CMD_FAIL			ERR_CMD_OK+1
#define ERR_CMD_NOT_SUPPORTED	ERR_CMD_OK+2
#define ERR_CMD_BAD_PARAMS		ERR_CMD_OK+3

/* Max command identifier size */
#define CMD_BUFFER_MAX 0x20

/* Command promt */
#define CMD_PROMT_INVITE ">> "

/* Pointer to loader descriptor */
static loader_descriptor_p loader_descriptor = 0;

static inline void tools_memory_dump(void *addr, word_t size)
{
	word_t i;
	for (i=0; i < size; ++i) {
		if (i != 0) 
			O(string," ");		

		O(string,"0x");
		if (((byte_t*)addr)[i] < 0x10)
			O(string,"0");

		O(number,((byte_t*)addr)[i],16);	
	}
}

static void tools_jump_to_kernel() 
{
	jump_to_kernel_asm();
}

static int tools_read_storage_data(disk_address_packet_t *address)
{
	int result = 0;
	
	if ( address->buffer & 0xFFFF0000 ) {
		/* upper memory */
		/* dst_seg is ignored, interpret address->buffer as linear 32bit address */
		/* store address */
		disk_address_packet_t stored_address;
		memcpy(&stored_address,address,sizeof(disk_address_packet_t));

		dword_t dst				= address->buffer;
		word_t needed_blocks	= address->blocks;

		address->buffer 		= IO_BUFFER_ADDRESS;
		address->blocks 		= 1;
		
		while (needed_blocks--) {

			result = BIOS_read_storage_data(address);
			if (result) {
				//O(char,'F');
				break;
			}
			//O(char,'.');

			copy_to_upper_memory_asm(dst,address->buffer,DISK_SECTOR_SIZE);
			address->lba	+= 1;
			dst 			+= DISK_SECTOR_SIZE;
		}

		/* restore address */
		memcpy(address,&stored_address,sizeof(disk_address_packet_t));
	}
	else {
		/* lower memory */
		result = BIOS_read_storage_data(address);
		if (result) {
			//O(char,'F');
		}
		//O(char,'.');
	}
	
	if (result) {
		O(string,"\r\nbuffer: 0x");
		O(number,address->buffer,16);
		O(string,"\r\nblocks: ");
		O(number,address->blocks,10);
		O(string,"\r\nlba: ");
		O(number,address->lba,10);
		O(string,"\r\n");
	}

	return result;
}

byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer)
{
	cmd_buffer = cmd_buffer;

	/* Load real mode kernel */

	/* 1. Load first KERNEL_SETUP_SECTORS sectors */
	disk_address_packet_t address;
	memset(&address,0,sizeof(address));
	address.struct_size	= sizeof(address);
	address.blocks		= KERNEL_SETUP_SECTORS;
	address.buffer		= KERNEL_SETUP_ADDRESS;
	address.lba			= KERNEL_CODE_LBA;
	O(string,"Loading kernel header ");
	if ( tools_read_storage_data(&address) ) {
		/* IO error */
		O(string," FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	O(string," DONE\r\n");

	/* 2. Check kernel signature */
	kernel_header_p kernel_header = GET_KERNEL_HEADER_ADDRESS(address.buffer);
	if ( kernel_header->header != KERNEL_HDRS ) {
		O(string,"Wrong kernel signature\r\n");
		return ERR_CMD_FAIL;
	}
	
	/* 3. Header analyze */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}
	
	O(string,"Supported LBP: 0x");
	O(number,kernel_header->version,16);
	O(string,"\r\n");

	/* 3. Load realmode kernel */
	address.blocks = kernel_header->setup_sects+1;
	address.buffer = KERNEL_REAL_CODE_ADDRESS;
	O(string,"Loading real mode kernel ");
	if ( tools_read_storage_data(&address) ) {
		/* IO error */
		O(string," FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	O(string," DONE\r\n");

	/* correct setup_sects */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}

	/* 4. Load protected mode kernel */
	address.lba 	+=	kernel_header->setup_sects + 1;
	address.blocks	=	loader_descriptor->kernel_sectors_count - (kernel_header->setup_sects + 1);	
	address.buffer	=	KERNEL_CODE_ADDRESS;
	O(string,"Loading protected mode kernel");
	if ( tools_read_storage_data(&address) ) {
		/* IO error */
		O(string," FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	O(string," DONE\r\n");

	return ERR_CMD_OK;
}

byte_t IMAGE_boot(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	O(string,"Boot...\r\n");
	
	tools_jump_to_kernel();

	return ERR_CMD_OK;
}

/* Display memory */
byte_t TOOLS_display_memory(byte_t *cmd_buffer) {
	
	strtok(" ", cmd_buffer);
	strtok(" ", 0);

	byte_t *addr_s = strtok(" ", 0);
	if (!addr_s)
		return ERR_CMD_BAD_PARAMS;

	dword_t addr = atol(addr_s, 16);
	
	byte_t *sz_s = strtok(" ", 0);
	if (!sz_s)
		return ERR_CMD_BAD_PARAMS;
	
	word_t sz = atol(sz_s, 10);	
	
	O(string,"address: 0x");
	O(number,addr,16);
	O(string,"\r\nsize: ");
	O(number,sz,10);
	O(string,"\r\n");
	tools_memory_dump(addr, sz);
	O(string,"\r\n");

	return ERR_CMD_OK;	
}

/* Command processor entry point */
byte_t C_process_command(byte_t *cmd_buffer) {
	byte_t r = ERR_CMD_NOT_SUPPORTED;

	if ( starts_from(cmd_buffer, "display") || starts_from(cmd_buffer, "d") ) {
		r = TOOLS_display_memory(cmd_buffer);
	}
	else
	if ( starts_from(cmd_buffer, "load") || starts_from(cmd_buffer, "l") ) {
		r = IMAGE_load_kernel_to_memory(cmd_buffer);
	}
	else
	if ( starts_from(cmd_buffer, "boot") || starts_from(cmd_buffer, "b") ) {
		r = IMAGE_boot(cmd_buffer);
	}

	return r;
}

/* Input loop callback */
byte_t C_input_cb(byte_t scancode, byte_t ascii) {
	
	static byte_t cmd_buffer[CMD_BUFFER_MAX];
	static byte_t *pos = cmd_buffer;

	if (scancode == VK_ENTER) {
		O(string,"\r\n");
		if ( pos != cmd_buffer ) {
			byte_t res = C_process_command(cmd_buffer);
			if (res) {
				// TODO: Out symbolic error code 
				O(string,"command error: ");
				O(number,res,16);
				O(string,"\r\n");
				
				//BIOS_reset(0x1234);
			}				
		}
		O(string,CMD_PROMT_INVITE);

		/* Reset buffer */
		pos		= cmd_buffer;
		*pos	= 0;
	}
	else {
		/* command buffer overflow protection */
		if ( (cmd_buffer + CMD_BUFFER_MAX) >= (pos + 1) ) {
			*pos	= ascii;
			*(pos+1)= 0;
			++pos;
		}
		O(char,ascii);
	}

	return 0;
}

/* Input loop callback */
byte_t C_no_input_cb(void) {
	return 0;
}

/* Halt system */
void C_stop(void) {
	 asm("cli\n" 
		 "hlt\n");
}

#include "gdt_table.h"

/* 16 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) {

	/* Calculate GDT address */
	init_gdt();
	
	/* Get console out interface */
	BIOS_init_console_out(out);

	/* Get loader descriptor information */
	loader_descriptor_p desc = (loader_descriptor_p)loader_descriptor_address;
	loader_descriptor = desc;
	
	/* Out information and command promt */
	O(string,"SS loader v");
	O(number,desc->version[0],10);
	O(char,'.');
	O(number,desc->version[1],10);
	O(char,'.');
	O(number,desc->version[2],10);
	O(string," (c)daemondzk@gmail.com");
	
	/* Memory map */
	O(string,"\r\nDescriptor: 0x");
	O(number,loader_descriptor_address,16);
	O(string,"\r\nCode: 0x");
	O(number,loader_code_address,16);
	O(string,"\r\nStack: 0x");
	O(number,LOADER_STACK_ADDRESS,16);
	O(string,"\r\nLoader sectors: 0x");
	O(number,desc->loader_sectors_count,16);
	O(string,"\r\nKernel sectors: 0x");
	O(number,desc->kernel_sectors_count,16);	
	O(string,"\r\nGDT size: ");
	O(number,gdtr,10);
	O(string,"\r\nGDT address: 0x");
	O(number,gdt_addr,16);

#ifdef CONFIG_SUPPORT_CMD_LINE

	O(string,"\r\n" CMD_PROMT_INVITE);

	/* Run main loop */
	BIOS_run_input_loop(C_input_cb,C_no_input_cb);

#else

	O(string,"\r\n");

	/* Boot linux image */
	if ( ERR_CMD_OK != IMAGE_load_kernel_to_memory(0) )	{
		O(string,"\r\nError during loading linux image");
	}
	else {
		IMAGE_boot(0);
	}

#endif // CONFIG_SUPPORT_CMD_LINE

	/* Stop */
	C_stop();

}

