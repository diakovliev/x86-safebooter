/*
 * mkimg.c
 *
 *  Created on: Jul 12, 2011
 *      Author: D.Iakovliev
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include <crypt.h>

static int verbose = 0;
static int decrypt = 0;
static char *input_file = 0;
static char *output_file = 0;
static void *buffer = 0;

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
		}
		fseek(input, 0, SEEK_SET);

		buffer = malloc(size);
		if (buffer && verbose) {
			printf("Allocated buffer at %p\n\r", buffer);
		} else
		if (!buffer){
			printf("Buffer allocation error\n\r");
			res = -1;
		}

		if (buffer) {
			do {
				readed += fread(buffer + readed, 1, size - readed, input);
				if (verbose) {
					printf("Readed %ld bytes\n\r", readed);
				}
			} while (readed < size);
		}

		fclose(input);
	}

	readed = 0;
	if (buffer) {
		FILE *output = fopen(output_file, "w+");
		if (output) {

			if (verbose) {
				printf(!decrypt?"Encryption...":"Decryption...");
			}
			blowfish_init();
			if (!decrypt) {
				blowfish_encrypt_memory(buffer, size);
			} else {
				blowfish_decrypt_memory(buffer, size);
			}
			if (verbose) {
				printf("DONE\n\r");
			}

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
	}

	return res;
}

void print_help(void) {
	printf("Usage:\tmkimg [--verbose] [-decrypt] -i input_file -o output_file\n\r");
}

int main(int argc, char **argv) {
	int res = 0;

	/* process arguments */
	static const struct option options[] = {
		{"verbose", no_argument, &verbose, 1},
		{"decrypt", no_argument, &decrypt, 1},
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

	if (!input_file) {
		printf("Unknown input file\n\r");
		print_help();
		res = 2;
	}
	if (!output_file) {
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

