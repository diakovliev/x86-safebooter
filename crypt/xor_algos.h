#ifndef XOR_ALGOS_HEADER
#define XOR_ALGOS_HEADER

#include <stdint.h>

void xor_encrypt_memory(void* buffer, uint32_t size);

void xor_scrambler_reset();
void xor_scramble_memory(void* buffer, uint32_t size);
void xor_descramble_memory(void* buffer, uint32_t size);

#endif/*XOR_ALGOS_HEADER*/
