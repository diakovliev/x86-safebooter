#include "crypt.h"

#include "blowfish.h"

extern byte_t *blowfish_key;
extern dword_t blowfish_key_size;

static BLOWFISH_CTX context;

void blowfish_init(void) {
	Blowfish_Init(&context, blowfish_key, blowfish_key_size);
}

void blowfish_encrypt_memory(void* buffer, dword_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array < buffer + size; array += 2) {
		Blowfish_Encrypt(&context, *array, *(array+1));
	}
}

void blowfish_decrypt_memory(void* buffer, dword_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array < buffer + size; array += 2) {
		Blowfish_Decrypt(&context, *array, *(array+1));
	}
}

#include "sha2.h"

void sha2_256(byte_t *digest, void *buffer, dword_t size) {
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, buffer, size);
	SHA256_Final(digest, &ctx);
}

#if 0
void sha2_348(byte_t *digest, void *buffer, dword_t size) {
	SHA348_CTX ctx;
	SHA348_Init(&ctx);
	SHA348_Update(&ctx, buffer, size);
	SHA348_Final(digest, &ctx);
}
#endif

void sha2_512(byte_t *digest, void *buffer, dword_t size) {
	SHA512_CTX ctx;
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, buffer, size);
	SHA512_Final(digest, &ctx);
}

