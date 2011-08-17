#include "ata_driver.h"
#include <loader.h>
#include <common.h>
#include <stdio.h>
#include <heap.h>

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
	else if (cl==0x14 && ch==0xeb)	devtype = ATADEV_PATAPI;
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
		__asm__ __volatile__(
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

void ata_enum_devices(ata_enum_callback callback, void *context) {

	byte_t res = 0;
	byte_t type = ATADEV_NONE;

#define ATA_ENUM(bus,drive) \
	type = ata_identify_device(bus, drive); \
	if (type != ATADEV_NONE) { \
		if (callback) res = (*callback)(bus, drive, type, context); \
		if (res) return; \
	}

	ATA_ENUM(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER);
	ATA_ENUM(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE);
	ATA_ENUM(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER);
	ATA_ENUM(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE);

#undef ATA_ENUM

}

/*---------------------------------------------------------------------------------------*/
typedef struct input_stream_context_s   {
	/* Drive */
	word_t bus;
	byte_t drive;

	/* Address */
	dword_t caddr;

} input_stream_context, *input_stream_context_p;

#ifdef CTX
#error "CTX already defined"
#endif
#define CTX ((input_stream_context_p)ctx)

word_t ata_read(byte_p dst, word_t size, void *ctx) {

	word_t res = ata_read_sectors(CTX->bus,CTX->drive,(void*)dst,size,CTX->caddr);
	CTX->caddr += res;

	return res;
}

dword_t ata_seek(dword_t addr, void *ctx) {

	dword_t res = CTX->caddr;

	CTX->caddr = addr;

	return res;
}

dword_t ata_addr(void *ctx) {

	return CTX->caddr;
}

#undef CTX

blk_istream_p ata_blk_stream_open(word_t bus, byte_t drive, dword_t addr) {

	blk_istream_p res = malloc(sizeof(blk_istream_t));
	if (res) {
		memset(res, 0, sizeof(blk_istream_p));
		res->read = ata_read;
		res->seek = ata_seek;
		res->addr = ata_addr;
		input_stream_context_p ctx = malloc(sizeof(input_stream_context));
		if (!ctx) {
			free(res);
			res = 0;
		} else {
			memset(ctx, 0, sizeof(input_stream_context));
			ctx->bus = bus;
			ctx->drive = drive;
			ctx->caddr = addr;
			res->ctx = ctx;
		}
	}

	return res;
}

void ata_blk_stream_close(blk_istream_p ptr) {
	if (ptr && ptr->ctx)
		free(ptr->ctx);
	if (ptr)
		free(ptr);
}
