//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include "common.h"
#include "txt_display.h"
#include "string.h"

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	display_t d;
	
	display_init(&d, TXT_VIDEO_MEM, 80, 25);
	display_clear(&d);
	
	display_puts(&d, "Initialize keyboard...\r\n");

	//kbd_init();

	byte_t scancode = 20;
	do {
		//scancode = kbd_get();
		display_puts(&d, "Pressed key: 0x");
		display_puts(&d, itoa(scancode,16));
		display_puts(&d, "\r\n");
	}
	while (scancode++);
}

