#ifndef ATA_DRIVER_HEADER
#define ATA_DRIVER_HEADER

#include <loader_types.h>

/* ATA bus */
#define ATA_BUS_PRIMARY				0x1F0
#define ATA_BUS_SECONDARY			0x170

/* ATA drives */
#define ATA_DRIVE_MASTER			0xA0
#define ATA_DRIVE_SLAVE				0xB0

/* ATA ports */
#define ATA_DATA_PORT(x)			(x+0)
#define ATA_FEATURES_PORT(x)		(x+1)
#define ATA_SECTORS_COUNT_PORT(x)	(x+2)
#define ATA_ADDRESS1_PORT(x)		(x+3)
#define ATA_ADDRESS2_PORT(x)		(x+4)
#define ATA_ADDRESS3_PORT(x)		(x+5)
#define ATA_DRIVE_SELECT_PORT(x)	(x+6)
#define ATA_COMMAND_PORT(x)			(x+7)
#define ATA_DCR_PORT(x)				(x+0x206)

/* ATA status */
#define ATA_ERR					(1<<0)
#define ATA_DRQ					(1<<3)
#define ATA_SRV					(1<<4)
#define ATA_DF					(1<<5)
#define ATA_RDY					(1<<6)
#define ATA_BSY					(1<<7)

/* Device control port & values */
#define ATA_DCR__nIEN		(1<<1)
#define ATA_DCR__SRST		(1<<2)
#define ATA_DCR__HOB		(1<<7)

#define ATA_SELECT_DELAY(bus) \
	{inb(ATA_DCR_PORT(bus));inb(ATA_DCR_PORT(bus));inb(ATA_DCR_PORT(bus));inb(ATA_DCR_PORT(bus));}

#define ATA_CMD_IDENTIFY	0xEC

/* Routes */
static inline byte_t ata_identify_device(word_t bus, byte_t drive) {

	outb(ATA_DRIVE_SELECT_PORT(bus),drive);
	outb(ATA_SECTORS_COUNT_PORT(bus),0);
	outb(ATA_ADDRESS1_PORT(bus),0);
	outb(ATA_ADDRESS2_PORT(bus),0);
	outb(ATA_ADDRESS3_PORT(bus),0);

	outb(ATA_COMMAND_PORT(bus),ATA_CMD_IDENTIFY);
	byte_t status = inb(ATA_COMMAND_PORT(bus));
	idle();
	if (!status) {
		/* Drive not exists, return zero */
		return 0;
	}
	while (status & ATA_BSY) {
		status = inb(ATA_COMMAND_PORT(bus));
		idle();
	}

	/* Checking for non ATA drive */
	byte_t addr2 = inb(ATA_ADDRESS2_PORT(bus));
	byte_t addr3 = inb(ATA_ADDRESS3_PORT(bus));
	if(addr2 && addr3) {
		/* Non ATA drive */
		return 2;
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
	} while (i++ <= 256);
	return 1;
}

#endif//ATA_DRIVER_HEADER

