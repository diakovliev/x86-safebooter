#ifndef ATA_DRIVER_HEADER
#define ATA_DRIVER_HEADER

#include <loader_types.h>
#include <stdio.h>

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

/* ATA device types */
#define ATADEV_NONE			0x00
#define ATADEV_PATA			0x01
#define ATADEV_SATA			0x02
#define ATADEV_PATAPI		0x03
#define ATADEV_SATAPI		0x04
#define ATADEV_UNKNOWN		0xFF

/* ATA commands */
#define ATA_CMD_IDENTIFY		0xEC
#define ATA_CMD_READ_SECTORS	0x20
#define ATA_CMD_WRITE_SECTORS	0x30
#define ATA_CMD_FLUSH			0xE7

/* Base */
byte_t ata_identify_device(word_t bus, byte_t drive);
word_t ata_read_sectors(word_t bus, byte_t drive, void *buffer, word_t sectors, dword_t addr);
word_t ata_write_sectors(word_t bus, byte_t drive, void *buffer, word_t sectors, dword_t addr);

/* Enumerate drives */
typedef byte_t (*ata_enum_callback)(word_t bus, byte_t drive, byte_t type, void *ctx);
void ata_enum_devices(ata_enum_callback callback, void *ctx);

/* stdio interface */
blk_istream_p ata_blk_stream_open(word_t bus, byte_t drive, dword_t addr);
void ata_blk_stream_close(blk_istream_p ptr);

#endif//ATA_DRIVER_HEADER

