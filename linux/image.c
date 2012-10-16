#include "image.h"
#include "../tools/lbp.h"
#include "jump_to_kernel.h"
#include <loader.h>
#include <string.h>
#include <env.h>
#include <heap.h>
#include <stdio.h>
#include <main/simg.h>

static size_t simg_initrd_size = 0;
static dword_t simg_end_address = 0;

byte_t image_parse_ata_address(image_ata_address_p address, byte_p presentation)
{	
	strtok(":", presentation);
	byte_p type_s = strtok(":", 0);
	if (!type_s) {
		goto finish;
	}
	address->type = *type_s;
	if (address->type != 'R' && address->type != 'S') {
		goto finish;
	}
	byte_p bus_s = strtok(":", 0);
	if (!bus_s) {
		goto finish;
	}
	address->bus = atol(bus_s,16);
	byte_p drive_s = strtok(":", 0);
	if (!drive_s) {
		goto finish;
	}
	address->drive = atol(drive_s,16);
	byte_p lba_s = strtok(":", 0);
	if (!lba_s) {
		goto finish;
	}
	address->lba = atol(lba_s,16);
	byte_p size_s = strtok(":", 0);
	if (!size_s && address->type == 'R') {
		goto finish;
	}
	address->size = atol(size_s,16);
	return 0;
finish:
	printf("%s: wrong presentation format\n\r", __FUNCTION__);
	return -1;
}

#ifdef CONFIG_RAW_IMAGES_ENABLED

byte_t image_load_raw(blk_iostream_p s, dword_t image_start, dword_t whole_image_sectors) {

	word_t res = 0;
	simg_end_address = 0;
	blk_seek(image_start,s);

	/* Load real mode kernel */

	/* 1. Load first KERNEL_SETUP_SECTORS sectors */
	puts("Loading kernel header ");
	res = blk_read((void*)KERNEL_REAL_CODE_ADDRESS,KERNEL_SETUP_SECTORS,s);
	if ( res != KERNEL_SETUP_SECTORS ) {
		/* IO error */
		puts(" FAIL\r\n");
		return -1;
	}
	puts(" DONE\r\n");

	/* 2. Check kernel signature */
	kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(KERNEL_REAL_CODE_ADDRESS);
	if ( kernel_header->header != KERNEL_HDRS ) {
		puts("Wrong kernel signature\r\n");
		return -2;
	}

	/* 3. Header analyze */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}

	printf("Supported LBP: 0x%x\r\n", kernel_header->version);

	blk_seek(image_start,s);

	/* 3. Load realmode kernel */
	puts("Loading real mode kernel ");
	res = blk_read((void*)KERNEL_REAL_CODE_ADDRESS,kernel_header->setup_sects+1,s);
	if ( res != kernel_header->setup_sects+1 ) {
		/* IO error */
		puts(" FAIL\r\n");
		return -3;
	}
	puts(" DONE\r\n");

	/* correct setup_sects */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}

	word_t sectors_count = whole_image_sectors - (kernel_header->setup_sects + 1);
	/* 4. Load protected mode kernel */
	puts("Loading protected mode kernel");
	res = blk_read((void*)KERNEL_CODE_ADDRESS,sectors_count,s);
	if ( res != sectors_count ) {
		/* IO error */
		printf("FAIL (sectors_count: %d, res: %d)\r\n", sectors_count, res);
		return -4;
	}

	simg_end_address = KERNEL_CODE_ADDRESS + (sectors_count * DISK_SECTOR_SIZE);
	simg_initrd_size = 0;

	puts(" DONE\r\n");

	return 0;
}

#endif//CONFIG_RAW_IMAGES_ENABLED

byte_t image_load_sig(blk_iostream_p s, dword_t image_start) {

	int res = 0;

	simg_end_address = 0;

	dword_t sz;

	blk_seek(image_start,s);

	puts("Loading kernel image... ");
	if (load_simg((void*)KERNEL_REAL_CODE_ADDRESS,s,&sz) < 0) {
		puts("FAIL\n\r");
		return 1;
	}

	res = load_simg((void*)KERNEL_CODE_ADDRESS,s,&sz);
	if (res > 0) {
		simg_end_address = KERNEL_CODE_ADDRESS + res;
		simg_initrd_size = 0;
		puts("DONE\n\r");
	}
	else {
		puts("FAIL\n\r");
	}

	return simg_end_address == 0;
}

#ifdef CONFIG_RAW_IMAGES_ENABLED

byte_t image_load_initrd_raw(blk_iostream_p s, dword_t image_start, dword_t whole_image_sectors) {
	
	


}

#endif//CONFIG_RAW_IMAGES_ENABLED

byte_t image_load_initrd_sig(blk_iostream_p s, dword_t image_start) {

	int res = 0;
	simg_initrd_size = 0;

	blk_seek(image_start,s);

	puts("Loading initrd image... ");
	if (load_simg((void*)simg_end_address,s,&simg_initrd_size) < 0) {
		puts("FAIL\n\r");
		return 1;
	}
	puts("DONE\n\r");

	return 0;
}

void image_boot(loader_descriptor_p desc) {

	printf("Booting kernel image... ");

	if (simg_end_address == 0) {
		printf("The kernel image not loaded.\n\r");
		return;
	}

	byte_t *cmdline = env_get("APPEND");
	kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(KERNEL_REAL_CODE_ADDRESS);

	dword_t heap_end = 0;
	void* base_ptr = 0;	/* base address for real-mode segment */

	if ( kernel_header->version >= 0x0200 ) {
		kernel_header->type_of_loader = 0xFF;
		if (simg_initrd_size) {
			printf("Found initrd... ");
			kernel_header->ramdisk_image = simg_end_address;
			kernel_header->ramdisk_size = simg_initrd_size;
		}

		/* Place command line just above initrd */
		heap_end = (dword_t)simg_end_address + simg_initrd_size;
		//printf("heap_end: 0x%X\r\n", heap_end);

		if ( kernel_header->version >= 0x0201 ) {
			kernel_header->heap_end_ptr = heap_end - 0x200;
			kernel_header->loadflags |= 0x80; /* CAN_USE_HEAP */
		}

		if (cmdline) {
			if ( kernel_header->version >= 0x0202 ) {
				kernel_header->cmd_line_ptr = (dword_t)(base_ptr + heap_end);
				strcpy((byte_t*)kernel_header->cmd_line_ptr, cmdline);
			} else {
				printf("Too old kernel boot protocol (0x%X), command line is not supported.\n\r", kernel_header->version);
			}
		}

		puts("jump to kernel, bye\r\n");

		/* Boot */
		jump_to_kernel_asm();

	} else {

		printf("Unsupported kernel boot protocol (0x%X). Probably very old kernel.\n\r", kernel_header->version);

	}

}

