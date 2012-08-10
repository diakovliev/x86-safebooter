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

/* ATA device types */
#define ATADEV_NONE			0x00
#define ATADEV_PATA			0x01
#define ATADEV_SATA			0x02
#define ATADEV_PATAPI		0x03
#define ATADEV_SATAPI		0x04
#define ATADEV_UNKNOWN		0xFF

byte_t ata_probe_devices(void);

/* Enumerate drives */
typedef byte_t (*ata_enum_callback)(word_t bus, byte_t drive, byte_t type, void *ctx);
void ata_enum_devices(ata_enum_callback callback, void *ctx);

/* stdio interface */
blk_iostream_p ata_blk_stream_open(word_t bus, byte_t drive, dword_t addr);
void ata_blk_stream_close(blk_iostream_p ptr);

#endif//ATA_DRIVER_HEADER

