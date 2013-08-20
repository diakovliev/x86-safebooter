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

typedef struct xor_scrambler_data_s {
	uint8_t *key;
	uint8_t b;
} xor_scrambler_data_t;

static xor_scrambler_data_t *get_scrambler_data()
{
	static xor_scrambler_data_t data = { 0, 0 };
	return &data;
}

void xor_scrambler_reset()
{
	get_scrambler_data()->key = xor_key;
	get_scrambler_data()->b = 0;
}

void xor_scramble_memory(void* buffer, uint32_t size) {
	xor_scrambler_data_t *data = get_scrambler_data();
	
	uint8_t *array = (uint8_t*)buffer;

	for ( ; array < (uint8_t*)(buffer+size); ++array) {

		*array = *array ^ (uint8_t)(*(data->key++) ^ data->b) ;

		data->b = *array;
		if (data->key > xor_key + xor_key_size - 1)
			data->key = xor_key;
	}
}

void xor_descramble_memory(void* buffer, uint32_t size) {
	xor_scrambler_data_t *data = get_scrambler_data();

	uint8_t b_next = 0;
	uint8_t *array = (uint8_t*)buffer;

	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		b_next = *array;

		*array = *array ^ (uint8_t)(*(data->key++) ^ data->b);

		data->b = b_next;
		if (data->key > xor_key + xor_key_size - 1)
			data->key = xor_key;
	}
}

