#ifndef XOR_SCRAMBLER_HEADER
#define XOR_SCRAMBLER_HEADER

#include <stdint.h>

typedef uint8_t (*get_next_key_byte_t)(void *scrambler_ctx);

typedef struct xor_scrambler_data_s {
	uint8_t *key;
	uint8_t *key_orig;
	uint32_t key_size;
	uint8_t *genkey_data;
	uint8_t *genkey_data_ptr;
	uint32_t genkey_size;
	uint32_t genkey_seed;
	uint8_t b;	
	get_next_key_byte_t get_next_key_byte;
} xor_scrambler_data_t;
   
uint8_t xor_scrambler_generate_next_key_byte(void *scrambler_data);
uint8_t xor_scrambler_get_next_key_byte(void *scrambler_data);

void init_xor_scrambler(xor_scrambler_data_t *data, 
						uint8_t *key, 
						uint32_t key_size, 
						get_next_key_byte_t get_next_key_byte,
						uint32_t genkey_size);
void reset_xor_scrambler(xor_scrambler_data_t *data);

/***************************************************************************/
/*	Gdir * Ginv = 1 */
/***************************************************************************/
void xor_scrambler_Gdir(xor_scrambler_data_t *data, void* buffer, uint32_t size);
void xor_scrambler_Ginv(xor_scrambler_data_t *data, void* buffer, uint32_t size);

#endif/*XOR_SCRAMBLER_HEADER*/

