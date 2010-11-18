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

void C_start(void) {

	BIOS_print_char('C');
	BIOS_print_char(':');
	BIOS_print_number(0xAA11,10);
	BIOS_print_char(':');

	/* Out hello */
	BIOS_print_string("Hello loader");

	C_stop();
}

