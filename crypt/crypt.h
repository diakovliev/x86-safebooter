#ifndef CRYPT_HEADER
#define CRYPT_HEADER

#include <stdint.h>

void blowfish_init(void);
void blowfish_encrypt_memory(void* buffer, uint32_t size);
void blowfish_decrypt_memory(void* buffer, uint32_t size);

void sha2_256(uint8_t *digest, void *buffer, uint32_t size);
void sha2_512(uint8_t *digest, void *buffer, uint32_t size);

void sha1(uint8_t *digest, void *buffer, uint32_t size);

void xor_encrypt_memory(void* buffer, uint32_t size);

#endif//CRYPT_HEADER

