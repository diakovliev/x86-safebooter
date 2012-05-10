#include <heap.h>
#include <string.h>
#include <loader.h>
#include <crypt/dsa.h>
#include <crypt/crypt.h>

#include "simg.h"

int load_simg(void *address, blk_iostream_p s) {

	byte_t res = 0;
	uint8_t processed_sha2[SHA2_SIZE/8];

	/* Allocate start block */
	void *start_block = malloc(DISK_SECTOR_SIZE);	
	if (!start_block) {
		puts("Unable to allocate start block buffer\n\r");
		return -1;
	}
	void *start_block_orig = start_block;

	/* Read start block */
	res = blk_read(start_block,1,s);
	if (res != 1) {
		puts("Unable to read start block\n\r");
		free(start_block_orig);
		return -2;
	}

	/* Check SIMG */
#ifdef CONFIG_SIMG_SIGNATURE_ENABLED
	dword_t simg = 0;
	memcpy(&simg, start_block, sizeof(simg));
	start_block += sizeof(simg);
	if (simg != 0x474D4953) {
		puts("Wrong SIMG signature\n\r");
		free(start_block_orig);
		return -3;
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
   		return -4;
   	}

#ifdef CONFIG_SIMG_BLOWFISH_ENCRYPTED
   	/* Decrypt buffer */
   	blowfish_reset();
   	blowfish_decrypt_memory(address+res,size);
#endif

#ifdef CONFIG_SIMG_XOR_SCRAMBLED
   	xor_descramble_memory(address+res,size);
#endif/*CONFIG_SIMG_XOR_SCRAMBLED*/

   	dsa_free(bch_sha2);
	dsa_free(dsa_r);
	dsa_free(dsa_s);
	
	free(start_block_orig);

	return size;
}

