//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

asm(".code16gcc");

#include "loader.h"
#include "bios_tools.h"

void C_start(void) __attribute__((noreturn));
void C_stop(void) __attribute__((noreturn)); 

void C_stop(void) { 
	 asm("cli\n" 
		 "hlt\n");
}

byte_t C_input_cb(byte_t scancode, byte_t ascii) {
	BIOS_print_number(scancode,10);
	BIOS_print_char(':');
	BIOS_print_char(ascii);
	BIOS_print_char(':');
	BIOS_print_number(ascii,16);
	BIOS_print_char('(');
	BIOS_print_number(ascii,10);
	BIOS_print_char(')');
	BIOS_print_string("\n\r");
	return 0;
}

byte_t C_no_input_cb(void) {
	return 0;
}

void C_start(void) {

	BIOS_print_string("ssloader>");
	BIOS_print_string("\n\r");
	BIOS_run_input_loop(C_input_cb,C_no_input_cb);

	C_stop();
}

