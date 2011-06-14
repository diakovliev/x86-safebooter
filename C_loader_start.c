//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include <common.h>
#include <drivers/text_display_driver.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <string.h>

byte_t key_handler(byte_t scancode, word_t mod, void *d)
{
	byte_t ascii = ascii_2ascii(scancode,mod);
	
	if (mod & KBD_MOD_KEY_UP)
		display_puts(d, "Released key: 0x");
	else
		display_puts(d, "Pressed key: 0x");
		
	display_puts(d, itoa(scancode,16));
	display_puts(d, "\r\n");
	display_puts(d, "ASCII : ");
	display_puts(d, itoa(ascii,16));
	display_puts(d, " '");
	display_putc(d, ascii);
	display_puts(d, "'\r\n");
	
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

/* Detect ATA */
void detect_ata_drive(word_t bus, byte_t drive, void *d) {

	display_puts(d, "ATA(");
	display_puts(d, itoa(bus,16));
	display_puts(d, ":");
	display_puts(d, itoa(drive,16));
	display_puts(d, ") - ");

	switch (ata_identify_device(bus,drive)) {
	case 0:
		display_puts(d, "none");
		break;
	case 1:
		display_puts(d, "found");
		break;
	case 2:
		display_puts(d, "non ATA");
		break;
	default:
		display_puts(d, "unknown result");
	}

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

	/* Detect ATA drives */

	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER, &d);
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE, &d);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER, &d);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE, &d);
	
	display_puts(&d, "Initialize keyboard...\r\n");

	res = keyboard_init(&k,&d);
	if (KEYBOARD_OK == res) {
		keyboard_run_input_loop(&k,key_handler,0,0);	
	}
}

