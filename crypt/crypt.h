#ifndef CRYPT_HEADER
#define CRYPT_HEADER

#include <stdint.h>

typedef struct encryptor_s {		
	void (*reset)(void);
	void (*encrypt)(void* buffer, uint32_t size);
	void (*decrypt)(void* buffer, uint32_t size);
} encryptor, *encryptor_p;

void blowfish_reset(void);
void blowfish_encrypt_memory(void* buffer, uint32_t size);
void blowfish_decrypt_memory(void* buffer, uint32_t size);

#if 0
void sha2_256(uint8_t *digest, void *buffer, uint32_t size);
void sha2_512(uint8_t *digest, void *buffer, uint32_t size);
#endif

void sha1(uint8_t *digest, void *buffer, uint32_t size);

#include "xor_algos.h"

#endif//CRYPT_HEADER

