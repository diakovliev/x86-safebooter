/*
 * xor.c
 *
 *  Created on: Sep 15, 2011
 *      Author: D.Iakovliev
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>

#include <xor_algos.h>

#define BUFFER_SIZE 512

FILE *get_input_file(char *file_name)
{
	FILE *result = 0;
	if (file_name) {
		result = fopen(file_name, "r");
	}
	else {
		result = stdin;
	}
	return result;	
}

FILE *get_output_file(char *file_name)
{
	FILE *result = 0;
	if (file_name) {
		result = fopen(file_name, "w");
	}
	else {
		result = stdout;
	}
	return result;	
}

void print_help(void) {
	printf("Usage:\txor -[xsd] -i input_file [-o output_file]\n\r");
}

int main(int argc, char **argv)
{
	int res = 0;

	char *input_file_name = 0;
	char *output_file_name = 0;
	
	char mode = 'x';

	/* process arguments */
	static const struct option options[] = {
		{"xor", no_argument, 0, 'x'},
		{"scramble", no_argument, 0, 's'},
		{"descramble", no_argument, 0, 'd'},
		{"input", required_argument, 0, 'i'},
		{"output", required_argument, 0, 'o'},
		{"help", no_argument, 0, 'h'},
		{0, 0, 0, 0},
	};

	int opt, sh;
	while ((sh = getopt_long(argc, argv, "i:o:hxsd", options, &opt)) != -1) {
		switch (sh) {
		case 'x':
		case 's':
		case 'd':
			mode = sh;
		break;
		case 'i':
		{
			if (!optarg) break;
			input_file_name = strdup(optarg);
		}
		break;
		case 'o':
		{
			if (!optarg) break;
			output_file_name = strdup(optarg);
		}
		break;
		default:
			print_help();
			res = -1;
			goto out;
		}
	}

	FILE *input = get_input_file(input_file_name);
	FILE *output = get_output_file(output_file_name);

	if (input && output) {

		char *buffer = (char*)malloc(BUFFER_SIZE);
		if (buffer)
		{
			memset(buffer, 0, BUFFER_SIZE);
			if (mode != 'x') xor_scrambler_reset();

			size_t readed = 0;

			do {			
				readed = fread(buffer, 1, BUFFER_SIZE, input);
				if (readed)
				{
					switch (mode) {
					case 'x':
						xor_encrypt_memory(buffer, readed);
					break;
					case 's':
						xor_scramble_memory(buffer, readed);
					break;
					case 'd':
						xor_descramble_memory(buffer, readed);
					break;
					default:
						res = -2;
						goto out;
					}
					fwrite(buffer, 1, readed, output);
				}
			} while (readed > 0);

			free(buffer);
		}
		else {
			res = -3;
			goto out;
		}

		fclose(input);
		fclose(output);
	}
	else
	{
		res = -1;
	} 		

out:
	if (output_file_name) free(output_file_name);
	if (input_file_name) free(input_file_name);
	return res;
}

