#include "xor_algos.h"
#include "xor_scrambler.h"

/***************************************************************************/
extern uint8_t xor_key[];
extern uint32_t xor_key_size;

static xor_scrambler_data_t scrambler_data;

/***************************************************************************/
void xor_encrypt_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		*array = *(array) ^ *(key++);
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

/***************************************************************************/
void xor_scrambler_reset()
{
	init_xor_scrambler(&scrambler_data, xor_key, xor_key_size, xor_scrambler_get_next_key_byte, 0);
/*	init_xor_scrambler(&scrambler_data, xor_key, xor_key_size, xor_scrambler_generate_next_key_byte, 16);*/
	reset_xor_scrambler(&scrambler_data);
}

/***************************************************************************/
void xor_scramble_memory(void* buffer, uint32_t size) 
{
	xor_scrambler_Gdir(&scrambler_data, buffer, size);
}

/***************************************************************************/
void xor_descramble_memory(void* buffer, uint32_t size) 
{
	xor_scrambler_Ginv(&scrambler_data, buffer, size);
}

