#include <string.h>

#include "sha1.h"
#include "blowfish.h"
#include "crypt.h"

/* Encryption modes */
/*#define CONFIG_BLOWFISH_MODE_ECB /* not recomended to use */
/*#define CONFIG_BLOWFISH_MODE_CTR_S /* simple inc counter */
#define CONFIG_BLOWFISH_MODE_CTR_D /* double inc counter */
/*#define CONFIG_BLOWFISH_MODE_CBC*/
/*#define CONFIG_BLOWFISH_MODE_CFB*/
/*#define CONFIG_BLOWFISH_MODE_OFB*/

extern uint8_t blowfish_key[];
extern uint32_t blowfish_key_size;

static BLOWFISH_CTX blowfish_context;

void blowfish_reset(void) {
	memset((void*)&blowfish_context, 0, sizeof(blowfish_context));
	Blowfish_Init(&blowfish_context, blowfish_key, blowfish_key_size);
}

#if defined(CONFIG_BLOWFISH_MODE_ECB)
#warning "Enabled ECB encryption mode, we are strongly recommend do not use this mode."

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Encrypt(&blowfish_context, array, (array+1));
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Decrypt(&blowfish_context, array, (array+1));
	}
}

#elif defined(CONFIG_BLOWFISH_MODE_CTR_S)

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = ctr ^ *array;
		a_1 = ctr ^ *(array + 1);
		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);
		*array = a_0;
		*(array + 1) = a_1;
		ctr++;
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = *array;
		a_1 = *(array + 1);
		Blowfish_Decrypt(&blowfish_context, &a_0, &a_1);
		*array = a_0 ^ ctr;
		*(array + 1) = a_1 ^ ctr;
		ctr++;
	}
}

#elif defined(CONFIG_BLOWFISH_MODE_CTR_D)

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = ctr++ ^ *array;
		a_1 = ctr++ ^ *(array + 1);
		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);
		*array = a_0;
		*(array + 1) = a_1;
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = *array;
		a_1 = *(array + 1);
		Blowfish_Decrypt(&blowfish_context, &a_0, &a_1);
		*array = a_0 ^ ctr++;
		*(array + 1) = a_1 ^ ctr++;
	}
}

#elif defined(CONFIG_BLOWFISH_MODE_CBC)

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = *array;
		a_1 = *(array + 1);

		a_0 ^= pa_0;
		a_1 ^= pa_1;

		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);

		pa_0 = a_0;
		pa_1 = a_1;

		*array = a_0;
		*(array + 1) = a_1;
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = *array;
		a_1 = *(array + 1);

		Blowfish_Decrypt(&blowfish_context, &a_0, &a_1);

		a_0 ^= pa_0;
		a_1 ^= pa_1;

		pa_0 = *array;
		pa_1 = *(array+1);

		*array = a_0;
		*(array + 1) = a_1;
	}
}

#elif defined(CONFIG_BLOWFISH_MODE_CFB)

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = pa_0;
		a_1 = pa_1;

		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;

		pa_0 = a_0;
		pa_1 = a_1;
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);

		pa_0 = *array;
		pa_1 = *(array + 1);

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;

		a_0 = pa_0;
		a_1 = pa_1;
	}
}

#elif defined(CONFIG_BLOWFISH_MODE_OFB)

void blowfish_encrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = pa_0;
		a_1 = pa_1;

		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);

		pa_0 = a_0;
		pa_1 = a_1;

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;
	}
}

void blowfish_decrypt_memory(void* buffer, uint32_t size) {
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		Blowfish_Encrypt(&blowfish_context, &a_0, &a_1);

		pa_0 = a_0;
		pa_1 = a_1;

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;

		a_0 = pa_0;
		a_1 = pa_1;
	}
}

#else
#error Blowfish encryption mode is not defined. Please define...
#endif

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

void sha1(uint8_t *digest, void *buffer, uint32_t size) {
	SHA_CTX ctx;
	SHAInit(&ctx);
	SHAUpdate(&ctx, buffer, size);
	SHAFinal(digest, &ctx);
}

