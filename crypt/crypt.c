#include "crypt.h"

#include "blowfish.h"

extern uint8_t blowfish_key[];
extern uint32_t blowfish_key_size;

static BLOWFISH_CTX context;

void blowfish_init(void) {
	Blowfish_Init(&context, blowfish_key, blowfish_key_size);
}

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Encrypt(&context, array, (array+1));
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Decrypt(&context, array, (array+1));
	}
}


#if 0
#include "sha2.h"

void sha2_256(uint8_t *digest, void *buffer, uint32_t size) {
	SHA256_CTX ctx;
	SHA256_Init(&ctx);
	SHA256_Update(&ctx, buffer, size);
	SHA256_Final(digest, &ctx);
}

void sha2_348(uint8_t *digest, void *buffer, uint32_t size) {
	SHA348_CTX ctx;
	SHA348_Init(&ctx);
	SHA348_Update(&ctx, buffer, size);
	SHA348_Final(digest, &ctx);
}

void sha2_512(uint8_t *digest, void *buffer, uint32_t size) {
	SHA512_CTX ctx;
	SHA512_Init(&ctx);
	SHA512_Update(&ctx, buffer, size);
	SHA512_Final(digest, &ctx);
}
#endif

#include "sha1.h"

void sha1(uint8_t *digest, void *buffer, uint32_t size) {
	SHA_CTX ctx;
	SHAInit(&ctx);
	SHAUpdate(&ctx, buffer, size);
	SHAFinal(digest, &ctx);
}

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

void xor_scramble_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	uint8_t b = 0;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		*array = *(array) ^ (uint8_t)(*(key++) + b) ;
		b = *array;
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

void xor_descramble_memory(void* buffer, uint32_t size) {
	uint8_t *key = xor_key;
	uint8_t *array = (uint8_t*)buffer;
	uint8_t b = 0, b_next = 0;
	for ( ; array < (uint8_t*)(buffer+size); ++array) {
		if (array > (uint8_t*)buffer) b_next = *array;
		*array = *(array) ^ (uint8_t)(*(key++) + b);
		b = b_next;
		if (key > xor_key + xor_key_size - 1)
			key = xor_key;
	}
}

