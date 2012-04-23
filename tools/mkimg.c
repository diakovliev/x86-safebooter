/*
 * mkimg.c
 *
 *  Created on: Jul 12, 2011
 *      Author: D.Iakovliev
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <time.h>

#include <crypt.h>
#include <bch.h>
#include <dsa.h>
#include <assert.h>

#include "lbp.h"

static int verbose = 0;

#define START_BLOCK_HEAD "SIMG"
#define DISK_SECTOR_SIZE 512
#define KERNEL_SETUP_SECTORS 4
#define FILL	0xFF

/*********************************************************************************/
void custom_random_init(void) {
	srand(time(NULL));
}

/*********************************************************************************/
bch_data custom_random(bch_data max) {
	bch_data r = rand();
	return (r % max) + (r & 1);
}

/*********************************************************************************/
void print_sha2(uint8_t *sha2) {
	int i = 0;
	for (i = 0; i < SHA2_SIZE/8; ++i) {
		printf("%02X", sha2[i]);
	}
}

/*********************************************************************************/
int process_buffer(void *buffer, long size, void *start_block) {

	int res = 0;
	void *start_block_ptr = start_block;

    uint8_t original_sha2[SHA2_SIZE/8];
    uint8_t processed_sha2[SHA2_SIZE/8];

	/* Random generator */
	bch_random randrom_gen = {
		.init	= custom_random_init,
		.random	= custom_random,
	};

	/* Init random generator */
	(*randrom_gen.init)();

	/* Randomize start block */
	bch_data_p start_block_randomize = (bch_data_p)start_block;
	bch_size i;
	for (i = 0; i < DISK_SECTOR_SIZE; ++i) {
		*start_block_randomize++ = (*randrom_gen.random)(0xFF);
	}

	/* Reinit random generator */
	(*randrom_gen.init)();

    /* Fill start block */

#ifdef CONFIG_SIMG_SIGNATURE_ENABLED
	uint32_t simg = SIMG_SIGNATURE;
    memcpy(start_block_ptr, &simg, sizeof(simg));
    start_block_ptr += sizeof(simg);
#endif/*CONFIG_SIMG_SIGNATURE_ENABLED*/

    memcpy(start_block_ptr, &size, sizeof(size));
    start_block_ptr += sizeof(size);

    printf("size: %ld\n\r", size);

    blowfish_init();

	if(verbose){
        printf("Encryption...");
    }
/*	xor_encrypt_memory(buffer, size);*/
	xor_scramble_memory(buffer, size);
    blowfish_encrypt_memory(buffer, size);
    if(verbose){
        printf("DONE\n\r");
    }

    /* SHA2 for processed buffer */
    SHA2_func(processed_sha2, buffer, size);
    if (verbose) {
    	printf("SHA2: ");
    	print_sha2(processed_sha2);
    	printf("\n\r");
    }

    bch_p bch_sha2 = dsa_from_ba((bch_data_p)processed_sha2, SHA2_SIZE/8);

	bch_p r =		dsa_alloc();
	bch_p s =		dsa_alloc();

#ifdef __DEBUG__
	if (verbose) {
		bch_p G =		dsa_from_ba((bch_data_p)dsa_G, dsa_G_size);
		bch_p P =		dsa_from_ba((bch_data_p)dsa_P, dsa_P_size);
		bch_p Q =		dsa_from_ba((bch_data_p)dsa_Q, dsa_Q_size);
		bch_p pub =		dsa_from_ba((bch_data_p)dsa_pub, dsa_pub_size);
		bch_p priv =	dsa_from_ba((bch_data_p)dsa_priv, dsa_priv_size);

		printf("#----------------- DSA params ---------------\n\r");
		bch_print("G = ", G);
		bch_print("P = ", P);
		bch_print("Q = ", Q);
		bch_print("pub = ", pub);
		bch_print("priv = ", priv);
		bch_print("sha2 = ", bch_sha2);
		printf("#--------------------------------------------\n\r");

		dsa_free(G);
		dsa_free(P);
		dsa_free(Q);
		dsa_free(pub);
		dsa_free(priv);
	}
#endif/*__DEBUG__*/

    dsa_sign(bch_sha2, r, s, &randrom_gen);
    if (verbose) {
    	bch_hprint("r", r);
    	bch_hprint("s", s);
    }
    res = dsa_check(bch_sha2, r, s);
    if (verbose) {
		bch_print("SHA2: ", bch_sha2);
    	printf("dsa_check result: %x\n\r", res);
    }

    /* Fill start block */
    memcpy(start_block_ptr, r->data, SHA2_SIZE/8);
    start_block_ptr += SHA2_SIZE/8;
    memcpy(start_block_ptr, s->data, SHA2_SIZE/8);
    start_block_ptr += SHA2_SIZE/8;

    dsa_free(r);
    dsa_free(s);
    dsa_free(bch_sha2);

    return res;
}

/*********************************************************************************/
int process_raw_file(char *output_file, char *input_file) {

	int res = -3;
	long size = 0;
	long alloc_size = 0;
	long readed = 0;
	void *buffer = 0;
	void *start_block = 0;

	FILE *input = fopen(input_file, "r");
	/* load file to memory */
	if (input) {
		fseek(input, 0, SEEK_END);
		size = ftell(input);
		alloc_size = ((size/4)+(size%4?1:0))*4;
		if (verbose) {
			printf("Input size: %ld bytes\n\r", size);
			printf("Allocated size: %ld bytes\n\r", alloc_size);
		}
		fseek(input, 0, SEEK_SET);

		buffer = malloc(alloc_size);
		memset(buffer,0,alloc_size);

		if (buffer && verbose) {
			printf("Allocated buffer at %p\n\r", buffer);
		} else
		if (!buffer){
			printf("Buffer allocation error\n\r");
			res = -1;
		}

		start_block = malloc(DISK_SECTOR_SIZE);
		memset(start_block,0,DISK_SECTOR_SIZE);

		if (buffer && verbose) {
			printf("Allocated start block at %p\n\r", start_block);
		} else
		if (!buffer){
			printf("Start block allocation error\n\r");
			res = -1;
		}

		if (buffer && start_block) {

			readed = 0;
			do {
				readed += fread(buffer + readed, 1, size - readed, input);
				if (verbose) {
					printf("Readed %ld bytes\n\r", readed);
				}
			} while (readed < size);

		}

		fclose(input);
	}

	/* Write output */
	if (buffer && start_block) {
		FILE *output = fopen(output_file, "w+");
		if (output) {

			res = process_buffer(buffer, size, start_block);
			if (!res) {
				readed = 0;
				do {
					readed += fwrite(start_block + readed, 1, DISK_SECTOR_SIZE - readed, output);
					if (verbose) {
						printf("Wrote %ld bytes\n\r", readed);
					}
				} while (readed < DISK_SECTOR_SIZE);

				readed = 0;
				do {
					readed += fwrite(buffer + readed, 1, size - readed, output);
					if (verbose) {
						printf("Wrote %ld bytes\n\r", readed);
					}
				} while (readed < size);

				fclose(output);
				res = 0;
			}
		} else {
			res = -2;
		}

		free(buffer);
		free(start_block);
	}

	return res;
}

/*********************************************************************************/
int process_kernel_file(char *output_file, char *input_file) {
	/* Kernel will be in 2 separated blocks */
	/* 1. Real mode kernel */
	/* 2. Protected mode kernel */

	int				res					= 0;
	uint32_t		whole_image_sectors = 0;
	uint8_t			setup_sects 		= 0;
	kernel_header_p	kernel_header 		= NULL;
	size_t 			size				= 0;
	void 			*start_block		= NULL;
	size_t			readed				= 0;
	size_t			written				= 0;

	/* Open kernel file */
	FILE *input = fopen(input_file, "r");
	if (!input) {
		printf("Unable to open file '%s'\n\r", input_file);
		res = -1;
		goto out;
	}

	/* Open output file */
	FILE *output = fopen(output_file, "w+");
	if (!output) {
		printf("Unable to open file '%s'\n\r", output_file);
		res = -1;
		goto out;
	}

	/* Allocate buffer for kernel header */
	void *buffer = malloc(DISK_SECTOR_SIZE*KERNEL_SETUP_SECTORS);
	if (!buffer) {
		printf("Unable to allocate buffer\n\r");
		res = -1;
		goto out;
	}

	/* Calculate whole image size in sectors count */
	fseek(input, 0, SEEK_END);
	whole_image_sectors = ftell(input);
	whole_image_sectors = (whole_image_sectors / DISK_SECTOR_SIZE)
			+ (whole_image_sectors % DISK_SECTOR_SIZE ? 1 : 0);
	fseek(input, 0, SEEK_SET);

	/* Read kernel header */
	size = DISK_SECTOR_SIZE*KERNEL_SETUP_SECTORS;
	readed = fread(buffer, size, 1, input);
	if (readed != 1) {
		printf("Unable to read kernel header\n\r");
		res = -1;
		goto out;
	}
	kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(buffer);

	/* Calculate parameters for loading real mode kernel */
	setup_sects = kernel_header->setup_sects + 1;
	size = DISK_SECTOR_SIZE*setup_sects;

	/* Realloc buffer for whole real mode kernel */
	buffer = realloc(buffer, size);
	if (!buffer) {
		printf("Unable to reallocate buffer\n\r");
		res = -1;
		goto out;
	}

	/* Seek to begin */
	fseek(input, 0, SEEK_SET);

	/* Buffer cleanup */
	memset(buffer,FILL,size);

	/* Read real mode kernel */
	size = DISK_SECTOR_SIZE * setup_sects;
	readed = fread(buffer, size, 1, input);
	if (readed != 1) {
		printf("Unable to read real mode kernel\n\r");
		res = -1;
		goto out;
	}

	/* Allocate buffer for start block */
	start_block = malloc(DISK_SECTOR_SIZE);
	memset(start_block,FILL,DISK_SECTOR_SIZE);

	/* Encrypt & Sign buffer */
	res = process_buffer(buffer,size,start_block);
	if (res != 0) {
		printf("Process buffer error\n\r");
		res = -1;
		goto out;
	}

	/* Write real mode kernel buffer */
	written = fwrite(start_block, DISK_SECTOR_SIZE, 1, output);
	written = fwrite(buffer, size, 1, output);

	/* Realloc buffer for whole protected mode kernel */
	size = (whole_image_sectors - setup_sects) * DISK_SECTOR_SIZE;
	buffer = realloc(buffer, size);
	if (!buffer) {
		printf("Unable to reallocate buffer\n\r");
		res = -1;
		goto out;
	}

	/* Cleanup buffers */
	memset(start_block,FILL,DISK_SECTOR_SIZE);
	memset(buffer,FILL,size);

	/* Read protected mode kernel */
	readed = fread(buffer, 1, size, input);

	/* Encrypt & Sign buffer */
	res = process_buffer(buffer,size,start_block);
	if (res != 0) {
		printf("Process buffer error\n\r");
		res = -1;
		goto out;
	}

	/* Write protected mode kernel buffer */
	written = fwrite(start_block, DISK_SECTOR_SIZE, 1, output);
	written = fwrite(buffer, size, 1, output);

out:
	if (start_block) free(start_block);
	if (buffer) free(buffer);
	if (output) fclose(output);
	if (input) fclose(input);

	return res;
}

/*********************************************************************************/
char file_is_kernel(char *input_file) {

	char res = 0;

	FILE *input = fopen(input_file, "r");

	void *buffer = malloc(DISK_SECTOR_SIZE*KERNEL_SETUP_SECTORS);
	if (buffer) {

		fread(buffer, DISK_SECTOR_SIZE*KERNEL_SETUP_SECTORS, 1, input);

		kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(buffer);
		if ( kernel_header->header == KERNEL_HDRS ) {
			res = 1;
		}

		free(buffer);
	}

	fclose(input);

	return res;
}

/*********************************************************************************/
void print_help(void) {
	printf("Usage:\tmkimg [--verbose] -i input_file -o output_file\n\r");
}

/*********************************************************************************/
int main(int argc, char **argv) {
	int res = 0;

	char *input_file = 0;
	char *output_file = 0;

	/* process arguments */
	static const struct option options[] = {
		{"verbose", no_argument, &verbose, 1},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0},
	};

	int opt, sh;
	while ((sh = getopt_long(argc, argv, "i:o:h", options, &opt)) != -1) {
		switch (sh) {
		case 0:
		break;
		case 1:
		break;
		case 'i':
		{
			if (!optarg) break;
			input_file = strdup(optarg);
		}
		break;
		case 'o':
		{
			if (!optarg) break;
			output_file = strdup(optarg);
		}
		break;
		default:
			print_help();
			res = 1;
		}
	}

	if (!res && !input_file) {
		printf("Unknown input file\n\r");
		print_help();
		res = 2;
	}
	if (!res && !output_file) {
		printf("Unknown output file\n\r");
		print_help();
		res = 3;
	}

	if (!res && verbose) {
		printf(	"Verbose mode enabled.\n\r"
				"Input: %s\n\r"
				"Output: %s\n\r"
				, input_file
				, output_file
		);
	}

	/* Detect input type */
	if (!file_is_kernel(input_file)) {
		printf("Creating RAW data image\n\r");
		res = process_raw_file(output_file, input_file);
	} else {
		printf("Creating kernel image\n\r");
		res = process_kernel_file(output_file, input_file);
	}

	if (input_file) free(input_file);
	if (output_file) free(output_file);

#ifdef __DEBUG__
	if (verbose) {
		bch__memory_usage();
	}
#endif

	return res;
}

