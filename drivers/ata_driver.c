#include "ata_driver.h"
#include <config.h>
#include <loader_types.h>
#include <common.h>
#include <stdio.h>
#include <heap.h>
#include <debug.h>

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

/* ATA commands */
#define ATA_CMD_IDENTIFY		0xEC
#define ATA_CMD_READ_SECTORS	0x20
#define ATA_CMD_WRITE_SECTORS	0x30
#define ATA_CMD_FLUSH			0xE7

#define _ATA_CHECK_ERROR(msg,ret) \
	do { \
		idle(); \
		status = inb(ATA_COMMAND_PORT(bus)); \
	} while ( !(status & ATA_DRQ) && (status & ATA_BSY) ); \
	if ( status & ATA_ERR ) { \
		DBG(if(msg) puts(msg);) \
		return ret; \
	}
	
static byte_t ata_identify_device(word_t bus, byte_t drive) {

	byte_t devtype = ATADEV_NONE;
	byte_t status = 0;

	/* Select device */
	outb(ATA_DRIVE_SELECT_PORT(bus),drive&(1<<4));
	ATA_SELECT_DELAY(bus);

	outb(ATA_SECTORS_COUNT_PORT(bus),0);
	outb(ATA_ADDRESS1_PORT(bus),0);
	outb(ATA_ADDRESS2_PORT(bus),0);
	outb(ATA_ADDRESS3_PORT(bus),0);

	/* Send command */
	outb(ATA_COMMAND_PORT(bus),ATA_CMD_IDENTIFY);
	_ATA_CHECK_ERROR(0,devtype);

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

/* Typedef */
typedef byte_t (*ata_io_func)(word_t, byte_t, void *, byte_t, dword_t);

/* 28 bit PIO IO */
static byte_t ata_read_sectors_internal(word_t bus, byte_t drive, void *buffer, byte_t sectors, dword_t addr) {

	byte_t status = 0;

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
	_ATA_CHECK_ERROR(0,0);

	/* Read sectors */
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

		_ATA_CHECK_ERROR("ATA_ERR read\n\r", i);
	}

	return sectors;
}

/* 28 bit PIO IO */
static byte_t ata_write_sectors_internal(word_t bus, byte_t drive, void *buffer, byte_t sectors, dword_t addr) {

	byte_t status = 0;

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
	outb(ATA_COMMAND_PORT(bus),ATA_CMD_WRITE_SECTORS);
	_ATA_CHECK_ERROR(0,0);

	/* Write sectors */
	word_t i = 0, j;
	for (i = 0; i < sectors; ++i) {

		j = 0;	
		do {
			outw(ATA_DATA_PORT(bus),((word_t*)buffer)[(i*(DISK_SECTOR_SIZE/2))+j]);
		} while (++j < (DISK_SECTOR_SIZE/2));
		
		_ATA_CHECK_ERROR("ATA_ERR write\n\r", i);
	}

	outb(ATA_COMMAND_PORT(bus),ATA_CMD_FLUSH);
	_ATA_CHECK_ERROR("ATA_ERR flush\n\r", i);

	return sectors;
}

/****************************************************************/
static word_t ata_io(word_t bus, byte_t drive, void *buffer, word_t sectors, dword_t addr, ata_io_func io_func) {
	word_t i;
	byte_t count = sectors / 0xff;
	byte_t finish = sectors % 0xff;

	for ( i = 0; i < count * 0xff; i += 0xff ) {		
		if ( !(*io_func)(bus,drive,buffer+(DISK_SECTOR_SIZE*i),0xff,addr+i) )
			break;
	}
	if (!count || i == count * 0xff) {
		i += (*io_func)(bus,drive,buffer+(DISK_SECTOR_SIZE*i),finish,addr+i);
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

static word_t ata_read(byte_p dst, word_t size, void *ctx) {

	word_t res = ata_io(CTX->bus,CTX->drive,(void*)dst,size,
		CTX->caddr,ata_read_sectors_internal);
	CTX->caddr += res;

	return res;
}

static word_t ata_write(byte_p src, word_t size, void *ctx) {

	word_t res = ata_io(CTX->bus,CTX->drive,(void*)src,size,
		CTX->caddr,ata_write_sectors_internal);
	CTX->caddr += res;

	return res;
}

static dword_t ata_seek(dword_t addr, void *ctx) {

	dword_t res = CTX->caddr;

	CTX->caddr = addr;

	return res;
}

static dword_t ata_addr(void *ctx) {

	return CTX->caddr;
}

#undef CTX

blk_iostream_p ata_blk_stream_open(word_t bus, byte_t drive, dword_t addr) {

	blk_iostream_p res = malloc(sizeof(blk_iostream_t));
	if (res) {
		memset(res, 0, sizeof(blk_iostream_p));
		res->read	= ata_read;
		res->write	= ata_write;
		res->seek	= ata_seek;
		res->addr	= ata_addr;
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

void ata_blk_stream_close(blk_iostream_p ptr) {
	if (ptr && ptr->ctx)
		free(ptr->ctx);
	if (ptr)
		free(ptr);
}

