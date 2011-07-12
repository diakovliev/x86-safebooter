#ifndef CRYPT_HEADER
#define CRYPT_HEADER

#include <stdint.h>

extern void blowfish_init(void);
extern void blowfish_encrypt_memory(void* buffer, uint32_t size);
extern void blowfish_decrypt_memory(void* buffer, uint32_t size);

extern void sha2_256(uint8_t *digest, void *buffer, uint32_t size);
extern void sha2_512(uint8_t *digest, void *buffer, uint32_t size);

#endif//CRYPT_HEADER

