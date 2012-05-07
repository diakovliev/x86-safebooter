#include "xor_algos.h"

extern uint8_t xor_key[];
extern uint32_t xor_key_size;

void xor_encrypt_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		*array = *(array) ^ *(key++);
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

void xor_scramble_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	uint8_t b = 0;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		*array = *(array) ^ (uint8_t)(*(key++) ^ b) ;
		b = *array;
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

void xor_descramble_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	uint8_t b = 0, b_next = 0;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		b_next = *array;
		*array = *(array) ^ (uint8_t)(*(key++) ^ b);
		b = b_next;
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

