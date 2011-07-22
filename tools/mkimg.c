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

#include <crypt.h>
#include <gmp.h>

static int verbose = 0;
static int decrypt = 0;
static char *input_file = 0;
static char *output_file = 0;
static void *buffer = 0;
static void *start_block = 0;

#define START_BLOCK_HEAD "KRIM"
#define DISK_SECTOR_SIZE 512

#define SHA2_SIZE 256
#define SHA2_func sha2_256

void print_sha2(uint8_t *sha2) {
	int i = 0;
	for (i = 0; i < SHA2_SIZE/8; ++i) {
		printf("%X", sha2[i]);
	}
}

void process_buffer(void *buffer, long size) {

	void *start_block_ptr = start_block;

    uint8_t original_sha2[SHA2_SIZE/8];
    uint8_t processed_sha2[SHA2_SIZE/8];

    /* Fill start block */
    memcpy(start_block_ptr, START_BLOCK_HEAD, strlen(START_BLOCK_HEAD));
    start_block_ptr += strlen(START_BLOCK_HEAD);
    memcpy(start_block_ptr, &size, sizeof(size));
    start_block_ptr += sizeof(size);

    /* SHA2 for original buffer */
    SHA2_func(original_sha2, buffer, size);
    if (verbose) {
    	printf("Original SHA2: ");
    	print_sha2(original_sha2);
    	printf("\n\r");
    }

    gmp_number_p gmp_sha2 = gmp_tools_number_from_ba(original_sha2, SHA2_SIZE/8, 16);
    if (verbose) {
    	printf("Original gmp SHA2: ");
    	gmp_tools_dump_number("%X", gmp_sha2, 16);
    	printf("\n\r");
    }
    gmp_tools_free_number(gmp_sha2);

    memcpy(start_block_ptr, original_sha2, SHA2_SIZE/8);
    start_block_ptr += SHA2_SIZE/8;

    /* Encrypt */
	//if(verbose){
    //    printf(!decrypt ? "Encryption..." : "Decryption...");
    //}
	if(verbose){
        printf("Encryption...");
    }
    blowfish_init();
    //if(!decrypt){
        blowfish_encrypt_memory(buffer, size);
    //}else{
    //    blowfish_decrypt_memory(buffer, size);
    //}
    if(verbose){
        printf("DONE\n\r");
    }

    /* SHA2 for processed buffer */
    SHA2_func(processed_sha2, buffer, size);
    if (verbose) {
    	printf("Processed SHA2: ");
    	print_sha2(processed_sha2);
    	printf("\n\r");
    }
    memcpy(start_block_ptr, processed_sha2, SHA2_SIZE/8);
    start_block_ptr += SHA2_SIZE/8;
}

int process_file(void) {
	int res = -3;
	long size = 0;
	long readed = 0;

	FILE *input = fopen(input_file, "r");
	/* load file to memory */
	if (input) {

		fseek(input, 0, SEEK_END);
		size = ftell(input);
		if (verbose) {
			printf("Input size: %ld bytes\n\r", size);
			printf("Allocated size: %ld bytes\n\r", size+size%2);
		}
		fseek(input, 0, SEEK_SET);

		buffer = malloc(size+size%2);
		memset(buffer,0,size+size%2);

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

	if (buffer && start_block) {
		FILE *output = fopen(output_file, "w+");
		if (output) {

			process_buffer(buffer, size);

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

		} else {
			res = -2;
		}

		free(buffer);
		free(start_block);
	}

	return res;
}

void print_help(void) {
//	printf("Usage:\tmkimg [--verbose] [-decrypt] -i input_file -o output_file\n\r");
	printf("Usage:\tmkimg [--verbose] -i input_file -o output_file\n\r");
}

int main(int argc, char **argv) {
	int res = 0;

	/* process arguments */
	static const struct option options[] = {
		{"verbose", no_argument, &verbose, 1},
/*		{"decrypt", no_argument, &decrypt, 1},*/
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

	if (!res) {
		res = process_file();
	}

	if (input_file) free(input_file);
	if (output_file) free(output_file);

	return res;
}

