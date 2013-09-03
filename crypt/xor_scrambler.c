#include "xor_scrambler.h"

/***************************************************************************/
static inline uint32_t genkey_rand(xor_scrambler_data_t *data)
{
	data->genkey_seed = data->genkey_seed * 1103515245 + 12345;
	return (uint32_t)(data->genkey_seed / 65536) % 32768;
}
            
/***************************************************************************/
static inline void genkey_srand(xor_scrambler_data_t *data, uint32_t seed)
{
	data->genkey_seed = seed;
}

/***************************************************************************/
static inline void init_genkey_data(xor_scrambler_data_t *data, uint32_t genkey_size)
{
	data->genkey_seed = 0;
	data->genkey_size = genkey_size;
	data->genkey_data = (uint8_t*)malloc(data->genkey_size);
	data->genkey_data_ptr = data->genkey_data;
}

/***************************************************************************/
static inline void reset_genkey_data(xor_scrambler_data_t *data)
{
	data->genkey_seed = 0;
	data->genkey_data_ptr = 0;
	if (data->genkey_data)
	{
		free(data->genkey_data);
		data->genkey_data = 0;
	}
}

/***************************************************************************/
inline uint8_t xor_scrambler_generate_next_key_byte(void *scrambler_data)
{	
	xor_scrambler_data_t *data = (xor_scrambler_data_t *)scrambler_data;

	if (!data->genkey_data)
	{
		init_genkey_data(data, data->genkey_size);
	}

	if (!data->genkey_data)
	{
		return xor_scrambler_get_next_key_byte(data);
	}

	if (data->genkey_data == data->genkey_data_ptr) {
		uint8_t initial_value = *data->key++;

		if (data->key > data->key_orig + data->key_size - 1)
			data->key = data->key_orig;

		/* reinit pseudo random generator and generate genkey_size bytes for the key */
		genkey_srand(data, initial_value);
		for ( ;data->genkey_data_ptr < data->genkey_data + data->genkey_size; ++data->genkey_data_ptr)
		{
			*data->genkey_data_ptr = (uint8_t)genkey_rand(data);
		}
		data->genkey_data_ptr = data->genkey_data;
	}

	uint8_t result = *data->genkey_data_ptr++;
	if (data->genkey_data_ptr > data->genkey_data + data->genkey_size)
		data->genkey_data_ptr = data->genkey_data;

	return result;
}

/***************************************************************************/
inline uint8_t xor_scrambler_get_next_key_byte(void *scrambler_data)
{
	xor_scrambler_data_t *data = (xor_scrambler_data_t *)scrambler_data;
	uint8_t result = *data->key++;

	if (data->key > data->key_orig + data->key_size - 1)
		data->key = data->key_orig;
	
	return result;
}

/***************************************************************************/
void init_xor_scrambler(xor_scrambler_data_t *data, uint8_t *key, uint32_t key_size, 
				get_next_key_byte_t get_next_key_byte, uint32_t genkey_size)
{	
	data->key 		= key;
	data->key_orig	= key;
	data->key_size	= key_size;
	data->b 		= 0;
	data->get_next_key_byte = get_next_key_byte;
	init_genkey_data(data, genkey_size);
}

/***************************************************************************/
void reset_xor_scrambler(xor_scrambler_data_t *data)
{	
	data->key 		= data->key_orig;
	data->b 		= 0;
	reset_genkey_data(data);
}

/***************************************************************************/
/*	Gdir * Ginv = 1 */
/***************************************************************************/
void xor_scrambler_Gdir(xor_scrambler_data_t *data, void* buffer, uint32_t size) {
	uint8_t *array = (uint8_t*)buffer;

	for ( ; array < (uint8_t*)(buffer+size); ++array) {

		*array ^= (*data->get_next_key_byte)(data) ^ data->b;

		data->b = *array;
	}
}

/***************************************************************************/
void xor_scrambler_Ginv(xor_scrambler_data_t *data, void* buffer, uint32_t size) {
	uint8_t b_next = 0;
	uint8_t *array = (uint8_t*)buffer;

	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		b_next = *array;

		*array ^= (*data->get_next_key_byte)(data) ^ data->b;

		data->b = b_next;
	}
}

