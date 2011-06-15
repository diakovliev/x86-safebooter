#include "ata_driver.h"
#include <loader.h>
#include <common.h>

#include "console_iface.h"

byte_t ata_identify_device(word_t bus, byte_t drive) {

	byte_t devtype = ATADEV_NONE;

	/* Select device */
	outb(ATA_DRIVE_SELECT_PORT(bus),drive&(1<<4));
	ATA_SELECT_DELAY(bus);

	outb(ATA_SECTORS_COUNT_PORT(bus),0);
	outb(ATA_ADDRESS1_PORT(bus),0);
	outb(ATA_ADDRESS2_PORT(bus),0);
	outb(ATA_ADDRESS3_PORT(bus),0);

	/* Send command */
	outb(ATA_COMMAND_PORT(bus),ATA_CMD_IDENTIFY);
	byte_t status = 0;
	do {
		idle();
		status = inb(ATA_COMMAND_PORT(bus));
	} while ( !(status & ATA_DRQ) && (status & ATA_BSY) );
	if ( !status ) {
		/* Error */
		return devtype;
	}

	/* Checking for ATA device type */
	byte_t cl = inb(ATA_ADDRESS2_PORT(bus));
	byte_t ch = inb(ATA_ADDRESS3_PORT(bus));
	if (cl==0 && ch == 0) 	  		devtype = ATADEV_PATA;
	else if (cl==0x3c && ch==0xc3)	devtype = ATADEV_SATA;
	else if (cl==0x14 && ch==0xEB)	devtype = ATADEV_PATAPI;
	else if (cl==0x69 && ch==0x96)	devtype = ATADEV_SATAPI;
	else 							devtype = ATADEV_UNKNOWN;
	if (devtype != ATADEV_PATA) {
		return devtype;
	}

	/* Waitng identify data */
	if ( status & ATA_ERR ) {
		while ( status & ATA_ERR ) {
			status = inb(ATA_COMMAND_PORT(bus));
			idle();
		}
	}
	
	/* Reading identify data from data port */
	word_t i = 0;	
	do {
		word_t data = inw(ATA_DATA_PORT(bus));		
	} while (i++ < 256);

	return devtype;
}

/* 28 bit PIO IO */
static byte_t ata_read_sectors_internal(word_t bus, byte_t drive, void *buffer, byte_t sectors, dword_t addr) {

	/* Select device */
	byte_t slavebit = drive==ATA_DRIVE_SLAVE?1:0;
	outb(ATA_DRIVE_SELECT_PORT(bus),(0xE0|(slavebit<<4)|((addr>>24)&0x0F)));
	ATA_SELECT_DELAY(bus);

	outb(ATA_SECTORS_COUNT_PORT(bus),sectors);
	outb(ATA_FEATURES_PORT(bus),0);
	outb(ATA_ADDRESS1_PORT(bus),addr);
	outb(ATA_ADDRESS2_PORT(bus),addr>>8);
	outb(ATA_ADDRESS3_PORT(bus),addr>>16);

	/* Send command */
	outb(ATA_COMMAND_PORT(bus),ATA_CMD_READ_SECTORS);
	byte_t status = 0;
	do {
		idle();
		status = inb(ATA_COMMAND_PORT(bus));
	} while ( !(status & ATA_DRQ) && (status & ATA_BSY) );
	if ( status & ATA_ERR ) {
		/* Error */
		return 0;
	}

	/* Read sector */
	word_t i = 0, j;
	for (i = 0; i < sectors; ++i) {

		/*j = 0;	
		do {
			((word_t*)buffer)[(i*(DISK_SECTOR_SIZE/2))+j] = inw(ATA_DATA_PORT(bus));
		} while (++j < (DISK_SECTOR_SIZE/2));
		*/

		/* input data */
		asm(
			"movl %0,%%ecx\n"
			"movl %1,%%edi\n"
			"movl %2,%%edx\n"
			"cld\n"
			"rep insw"
			: /*no output*/
			: "g" (DISK_SECTOR_SIZE/2), "g" (buffer+(i*DISK_SECTOR_SIZE)), "g" (ATA_DATA_PORT(bus))
			: "ecx", "edi", "edx"
		);

		do {
			idle();
			status = inb(ATA_COMMAND_PORT(bus));
		} while ( !(status & ATA_DRQ) && (status & ATA_BSY) );
		if ( status & ATA_ERR ) {
			puts("ATA_ERR\n\r");
			// Error 
			return i;
		}
	}

	return sectors;
}

word_t ata_read_sectors(word_t bus, byte_t drive, void *buffer, word_t sectors, dword_t addr) {
	word_t i;
	byte_t count = sectors / 0xff;
	byte_t finish = sectors % 0xff;

	for ( i = 0; i < count * 0xff; i += 0xff ) {		
		if ( !ata_read_sectors_internal(bus,drive,buffer+(DISK_SECTOR_SIZE*i),0xff,addr+i) )
			break;
	}
	if (!count || i == count * 0xff) {
		i += ata_read_sectors_internal(bus,drive,buffer+(DISK_SECTOR_SIZE*i),finish,addr+i);
	}

	return i;
}
