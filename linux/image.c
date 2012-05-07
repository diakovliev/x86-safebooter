#include "image.h"
#include "../tools/lbp.h"
#include "jump_to_kernel.h"
#include <loader.h>
#include <string.h>
#include <env.h>
#include <heap.h>
#include <stdio.h>
#include <drivers/ata_driver.h>
#include <crypt/dsa.h>

static void *simg_end_address = 0;

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

	puts(" DONE\r\n");

	return 0;
}

#endif//CONFIG_RAW_IMAGES_ENABLED

byte_t image_load_simg_block(void *address, blk_iostream_p s) {

	byte_t res = 0;
	simg_end_address = 0;
	uint8_t processed_sha2[SHA2_SIZE/8];

	/* Allocate start block */
	void *start_block = malloc(DISK_SECTOR_SIZE);	
	if (!start_block) {
		puts("Unable to allocate start block buffer\n\r");
		return 1;
	}
	void *start_block_orig = start_block;

	/* Read start block */
	res = blk_read(start_block,1,s);
	if (res != 1) {
		puts("Unable to read start block\n\r");
		free(start_block_orig);
		return 2;
	}

	/* Check SIMG */
#ifdef CONFIG_SIMG_SIGNATURE_ENABLED
	dword_t simg = 0;
	memcpy(&simg, start_block, sizeof(simg));
	start_block += sizeof(simg);
	if (simg != 0x474D4953) {
		puts("Wrong SIMG signature\n\r");
		free(start_block_orig);
		return 3;
	}
#endif/*CONFIG_SIMG_SIGNATURE_ENABLED*/

	/* Image size */
	dword_t size = 0;
	memcpy(&size, start_block, sizeof(size));
	start_block += sizeof(size);

	bch_p dsa_r = dsa_alloc();
	bch_p dsa_s = dsa_alloc();

	memcpy(dsa_r->data, start_block, SHA2_SIZE/8);
    start_block += SHA2_SIZE/8;
	memcpy(dsa_s->data, start_block, SHA2_SIZE/8);
    start_block += SHA2_SIZE/8;

	/* Size in sectors */
	dword_t sectors = (size / DISK_SECTOR_SIZE) + (size % DISK_SECTOR_SIZE ? 1 : 0);
	memset(address,0xFF,size);
	res = blk_read(address,sectors,s);

	/* Calculate SHA2 */
	memset(processed_sha2,0,SHA2_SIZE/8);
    SHA2_func(processed_sha2,address,size);

    /* Check signing */
   	bch_p bch_sha2 = dsa_from_ba((bch_data_p)processed_sha2, SHA2_SIZE/8);
   	res = dsa_check(bch_sha2, dsa_r, dsa_s);
   	if (res) {
   		puts("Wrong digital signature\n\r");
		free(start_block_orig);
   		return 4;
   	}

   	/* Decrypt buffer */
   	blowfish_reset();
   	blowfish_decrypt_memory(address+res,size);

#ifdef CONFIG_SIMG_XOR_SCRAMBLED
   	xor_descramble_memory(address+res,size);
#endif/*CONFIG_SIMG_XOR_SCRAMBLED*/

	simg_end_address = address + res + size;

   	dsa_free(bch_sha2);
	dsa_free(dsa_r);
	dsa_free(dsa_s);
	
	free(start_block_orig);

	return 0;
}

byte_t image_load_sig(blk_iostream_p s, dword_t image_start) {

	byte_t res = 0;

	blk_seek(image_start,s);

	puts("Loading image... ");
	res |= image_load_simg_block((void*)KERNEL_REAL_CODE_ADDRESS,s);
	if (res) {
		puts("FAIL\n\r");
		return res;
	}

	res |= image_load_simg_block((void*)KERNEL_CODE_ADDRESS,s);
	if (!res) puts("DONE\n\r");
	else puts("FAIL\n\r");

	return res;
}

void image_boot(loader_descriptor_p desc) {

	printf("Booting image... ");

	if (simg_end_address == 0) {
		printf("The kernel image not loaded.\n\r");
		return;
	}

	kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(KERNEL_REAL_CODE_ADDRESS);

	dword_t heap_end = 0;
	void* base_ptr = 0;	/* base address for real-mode segment */

	if ( kernel_header->version >= 0x0200 ) {
		kernel_header->type_of_loader = 0xFF;
		//if ( loading_initrd ) {
		//	kernel_header->ramdisk_image = <initrd_address>;
		//	kernel_header->ramdisk_size = <initrd_size>;
		//}

		/* Place command line just above loader code */
		if ( kernel_header->version >= 0x0202 && kernel_header->loadflags & 0x01 )
			//heap_end = 0xe000;
			//heap_end = LOADER_CODE_ADDRESS + (desc->loader_sectors_count * DISK_SECTOR_SIZE);
			heap_end = (dword_t)simg_end_address;
		else
			//heap_end = 0x9800;
			//heap_end = LOADER_CODE_ADDRESS + (desc->loader_sectors_count * DISK_SECTOR_SIZE);
			heap_end = (dword_t)simg_end_address;

		//printf("heap_end: 0x%X\r\n", heap_end);

		if ( kernel_header->version >= 0x0201 ) {
			kernel_header->heap_end_ptr = heap_end - 0x200;
			kernel_header->loadflags |= 0x80; /* CAN_USE_HEAP */
		}

		byte_t *cmdline = env_get("APPEND");
		if (cmdline) {
			if ( kernel_header->version >= 0x0202 ) {
				kernel_header->cmd_line_ptr = (dword_t)(base_ptr + heap_end);
				strcpy((byte_t*)kernel_header->cmd_line_ptr, cmdline);
			} else {
				printf("Too old kernel boot protocol (0x%X), command line is not supported.\n\r", kernel_header->version);
				//kernel_header->cmd_line_magic	= 0xA33F;
				//kernel_header->cmd_line_offset = heap_end;
				//kernel_header->setup_move_size = heap_end + strlen(cmdline)+1;
				//strcpy(base_ptr+cmd_line_offset, cmdline);
			}
		}

		puts("jump to kernel, bye\r\n");

		/* Boot */
		jump_to_kernel_asm();

	} else {

		printf("Unsupported kernel boot protocol (0x%X). Probably very old kernel.\n\r", kernel_header->version);

	}

}
