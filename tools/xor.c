/*
 * xor.c
 *
 *  Created on: Sep 15, 2011
 *      Author: D.Iakovliev
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

extern uint8_t mbr_xor_key[];
extern uint32_t mbr_xor_key_size;

void xor_encrypt_memory(void* buffer, uint32_t size) {
	uint8_t *key = mbr_xor_key;
	uint8_t *array = (uint8_t*)buffer;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		*array = *(array) ^ *(key++);
		if (key > mbr_xor_key + mbr_xor_key_size - 1)
			key = mbr_xor_key;
	}
}

int main(int argc, char **argv) {
	int res = 0;

	if (argc < 2)
		return 1;

	FILE *f = NULL;
	char *fname = argv[1];
	f = fopen(fname, "r");
	if (!f) {
		perror(argv[0]);
		return 2;
	}

	long size;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);

	void *buffer = malloc(size);
	res = 3;
	if (buffer) {
		res = 0;
		fread(buffer, 1, size, f);
		fclose(f);
		fseek(f, 0, SEEK_SET);
		xor_encrypt_memory(buffer,size);
		f = fopen(fname, "w");
		if (!f) {
			perror(argv[0]);
			return 4;
		} else {
			fwrite(buffer, 1, size, f);
		}
	}

	free(buffer);
	fclose(f);

	return res;
}
