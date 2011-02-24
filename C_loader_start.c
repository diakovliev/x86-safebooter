//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

asm(".code16gcc");

#include "loader.h"
#include "bios_tools.h"

/* TODO: I have to find header with VK codes */
#define VK_ENTER 0x1C

/* TODO: Move to command processor header */
#define ERR_CMD_OK				0
#define ERR_CMD_NOT_SUPPORTED	ERR_CMD_OK+1

/* Max command identifier size */
#define CMD_BUFFER_MAX 0x20

/* Command promt */
#define CMD_PROMT_INVITE ">> "

/* Command processor entry point */
byte_t C_process_command(byte_t *cmd_buffer) {

	BIOS_print_string("command: ");
	BIOS_print_string(cmd_buffer);
	BIOS_print_string("\r\n");

	return ERR_CMD_NOT_SUPPORTED;
}

/* Input loop callback */
byte_t C_input_cb(byte_t scancode, byte_t ascii) {
	
	static byte_t cmd_buffer[CMD_BUFFER_MAX];
	static byte_t *pos = cmd_buffer;

	if (scancode == VK_ENTER) {
		BIOS_print_string("\r\n");
		if ( pos != cmd_buffer ) {
			byte_t res = C_process_command(cmd_buffer);
			if (res) {
				// TODO: Out symbolic error code 
				BIOS_print_string("command error: ");
				BIOS_print_number(res, 16);
				BIOS_print_string("\r\n");
				
				BIOS_reset(0x1234);
			}				
		}
		BIOS_print_string(CMD_PROMT_INVITE);

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
		BIOS_print_char(ascii);
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

/* 16 bit C code entry point */
void C_start(void *loader_descriptor_address) {

	/* Get loader descriptor information */
	loader_descriptor_t *desc = (loader_descriptor_t*)loader_descriptor_address;
	
	/* Out information and command promt */
	BIOS_print_string("SS loader v");
	BIOS_print_number(desc->version[0],10);
	BIOS_print_char('.');
	BIOS_print_number(desc->version[1],10);
	BIOS_print_char('.');
	BIOS_print_number(desc->version[2],10);
	BIOS_print_string(" (c)daemondzk@gmail.com\r\n");
	BIOS_print_string(CMD_PROMT_INVITE);

	/* Run main loop */
	BIOS_run_input_loop(C_input_cb,C_no_input_cb);

	/* Stop */
	C_stop();

}

