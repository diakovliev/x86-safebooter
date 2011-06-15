//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
#include <common.h>
#include <string.h>

#include <drivers/console_iface.h>
#include <drivers/text_display_driver.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <drivers/serial_driver.h>

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
	case ATADEV_NONE:
		display_puts(d, "none");
		break;
	case ATADEV_PATA:
		display_puts(d, "PATA");
		break;
	case ATADEV_SATA:
		display_puts(d, "SATA");
		break;
	case ATADEV_PATAPI:
		display_puts(d, "PATAPI");
		break;
	case ATADEV_SATAPI:
		display_puts(d, "SATAPI");
		break;
	default:
		display_puts(d, "Unknown");
	}

	display_puts(d, "\r\n");
}

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	loader_descriptor_p descriptor = (loader_descriptor_p)loader_descriptor_address;
	
	word_t serial_port = COM1;
	byte_t res = 0;
	display_t d;
	keyboard_driver_t k;
	
	display_init(&d, (void*)TXT_VIDEO_MEM, 80, 25);
	display_clear(&d);

	/* Detect ATA drives */
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER, &d);
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE, &d);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER, &d);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE, &d);

	/* Read */
	res = ata_read_sectors(ATA_BUS_PRIMARY, 
		ATA_DRIVE_MASTER, 
		(void*)KERNEL_CODE_ADDRESS, 
		(byte_t)descriptor->loader_descriptor_sectors_count, 
		LOADER_DESCRIPTOR_LBA); 

	loader_descriptor_p loaded_descriptor = (loader_descriptor_p)KERNEL_CODE_ADDRESS;
	
	if (res) {
		display_puts(&d, "Descriptor magic: ");
		display_puts(&d, itoa(descriptor->magic,16) );
		display_puts(&d, "\n\r");
		display_puts(&d, "Loaded descriptor magic: ");
		display_puts(&d, itoa(loaded_descriptor->magic,16) );
	} else {
		display_puts(&d, "ATA read error");
	}
	display_puts(&d, "\n\r");

	display_puts(&d, LOADER_ENV(loaded_descriptor));
	display_puts(&d, "\n\r");

	/* Initialize keyboard */	
/*	display_puts(&d, "Initialize keyboard...\r\n");
	res = keyboard_init(&k,&d);
	if (KEYBOARD_OK == res) {
		keyboard_run_input_loop(&k,key_handler,0,0);	
	}*/

	ser_init(serial_port);
	ser_write_string(serial_port, "This is the first serial communication\r\n");
	console_init(ser_get_console(serial_port));

	puts(">> ");
	byte_t c;
	while (1) {
		c = getc();
		if (c != '\r') {
			putc(c);
			display_putc(&d, c);
		} else {
			puts("\n\r");
			display_puts(&d, "\n\r");
		}
	}

}

