//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include "common.h"
#include "txt_display.h"
#include "keyboard.h"
#include "string.h"

byte_t key_handler(byte_t scancode, word_t mod, void *d)
{
//	display_clear(d);

	if (mod & KBD_MOD_KEY_UP)
		display_puts(d, "Released key: 0x");
	else
		display_puts(d, "Pressed key: 0x");
		
	display_puts(d, itoa(scancode,16));
	display_puts(d, "\r\n");
	
	display_puts(d, "Modifiers: ");
	if (mod & KBD_MOD_LSHIFT)
		display_puts(d, "LSHIFT ");		
	if (mod & KBD_MOD_RSHIFT)
		display_puts(d, "RSHIFT ");
	if (mod & KBD_MOD_LCTRL)
		display_puts(d, "LCTRL ");
	if (mod & KBD_MOD_RCTRL)
		display_puts(d, "RCTRL ");
	if (mod & KBD_MOD_LALT)
		display_puts(d, "LALT ");
	if (mod & KBD_MOD_RALT)
		display_puts(d, "RALT ");
	if (mod & KBD_MOD_CAPS)
		display_puts(d, "CAPS ");
	if (mod & KBD_MOD_NUM)
		display_puts(d, "NUM ");
	if (mod & KBD_MOD_SCROLL)
		display_puts(d, "SCROLL ");
	display_puts(d, "\r\n");

}

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	byte_t res = 0;
	display_t d;
	keyboard_driver_t k;
	
	display_init(&d, TXT_VIDEO_MEM, 80, 25);
	display_clear(&d);
	
	display_puts(&d, "Initialize keyboard...\r\n");

	res = keyboard_driver_init(&k,&d);
	if (KEYBOARD_OK == res) {
		keyboard_driver_run_input_loop(&k,key_handler,0,0);	
	}
}

