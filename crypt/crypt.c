#include "crypt.h"
#include "blowfish.h"

#define BLOWFISH_BLOCK_SIZE 64

extern byte_t *blowfish_key;
extern dword_t blowfish_key_size;

static BLOWFISH_CTX context;

void crypt_init(void) {
	Blowfish_Init(&context, blowfish_key, blowfish_key_size);
}

void encrypt_memory(void* buffer, dword_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array < buffer + size; array += 2) {
		Blowfish_Encrypt(&context, *array, *(array+1));
	}
}

void decrypt_memory(void* buffer, dword_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array < buffer + size; array += 2) {
		Blowfish_Decrypt(&context, *array, *(array+1));
	}
}

