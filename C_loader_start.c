//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
#include <common.h>
#include <string.h>

#include <drivers/console_iface.h>
#include <drivers/text_display_driver.h>
#include <drivers/text_display_console.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <drivers/serial_driver.h>

#if 0
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
#endif

/* Detect ATA */
void detect_ata_drive(word_t bus, byte_t drive) {

	puts("ATA(");
	puts(itoa(bus,16));
	puts(":");
	puts(itoa(drive,16));
	puts(") - ");

	switch (ata_identify_device(bus,drive)) {
	case ATADEV_NONE:
		puts("none");
		break;
	case ATADEV_PATA:
		puts("PATA");
		break;
	case ATADEV_SATA:
		puts("SATA");
		break;
	case ATADEV_PATAPI:
		puts("PATAPI");
		break;
	case ATADEV_SATAPI:
		puts("SATAPI");
		break;
	default:
		puts("Unknown");
	}

	puts("\r\n");
}

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	loader_descriptor_p descriptor = (loader_descriptor_p)loader_descriptor_address;
	
	byte_t res = 0;
	display_t d;
	keyboard_driver_t k;
	
	display_init(&d, (void*)TXT_VIDEO_MEM, 80, 25);
	keyboard_init(&k);
	display_clear(&d);
	
	//word_t serial_port = COM1;
	//ser_init(serial_port);
	//ser_write_string(serial_port, "This is the first serial communication\r\n");
	//console_init(ser_get_console(serial_port));
	
	console_init(display_get_console(&d,&k));

	/* Detect ATA drives */
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE);

	/* Read */
	res = ata_read_sectors(ATA_BUS_PRIMARY, 
		ATA_DRIVE_MASTER, 
		(void*)KERNEL_CODE_ADDRESS, 
		(byte_t)descriptor->loader_descriptor_sectors_count, 
		LOADER_DESCRIPTOR_LBA); 

	loader_descriptor_p loaded_descriptor = (loader_descriptor_p)KERNEL_CODE_ADDRESS;
	
	if (res) {
		puts("Descriptor magic: ");
		puts(itoa(descriptor->magic,16) );
		puts("\n\r");
		puts("Loaded descriptor magic: ");
		puts(itoa(loaded_descriptor->magic,16) );
	} else {
		puts("ATA read error");
	}
	puts("\n\r");

	puts(LOADER_ENV(loaded_descriptor));
	puts("\n\r");

	puts(">> ");
	byte_t c;
	while (1) {
		c = getc();
		if (c != '\r') {
			putc(c);
		} else {
			puts("\n\r>> ");
		}
	}

}

