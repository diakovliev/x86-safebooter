#ifndef CRYPT_HEADER
#define CRYPT_HEADER

#include <loader_types.h>

extern void blowfish_init(void);
extern void blowfish_encrypt_memory(void* buffer, dword_t size);
extern void blowfish_decrypt_memory(void* buffer, dword_t size);

extern void sha2_256(byte_t *digest, void *buffer, dword_t size);
extern void sha2_512(byte_t *digest, void *buffer, dword_t size);

#endif//CRYPT_HEADER

