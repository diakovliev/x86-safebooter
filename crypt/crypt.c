#include <string.h>

#include "sha1.h"
#include "blowfish.h"
#include "crypt.h"

/* Encryption modes */
#define CONFIG_BLOWFISH_MODE_ECB /* not recomended to use */
#define CONFIG_BLOWFISH_MODE_CTR_S /* simple inc counter */
#define CONFIG_BLOWFISH_MODE_CTR_D /* double inc counter */
#define CONFIG_BLOWFISH_MODE_CBC
#define CONFIG_BLOWFISH_MODE_CFB
#define CONFIG_BLOWFISH_MODE_OFB

#define CONFIG_BLOWFISH_VARIANT "blowfish_ctr_d"

extern uint8_t blowfish_key[];
extern uint32_t blowfish_key_size;

/**************************************************************************/
typedef struct blowfish_variant_ctx_s {
	BLOWFISH_CTX context;
	uint32_t ksize;
	uint8_t *key;
    void (*init)(void *ctx, uint8_t *key, uint32_t ksize);
	void (*reset)(void *ctx);
	void (*encrypt)(void *ctx, void* buffer, uint32_t size);
	void (*decrypt)(void *ctx, void* buffer, uint32_t size);
} blowfish_variant_ctx_t;

/**************************************************************************/
void blowfish_init_ctx(void *ctx, uint8_t *key, uint32_t ksize)
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	memset((void*)&variant->context, 0, sizeof(BLOWFISH_CTX));
	variant->key   = key;
	variant->ksize = ksize;
}

/**************************************************************************/
void blowfish_reset_ctx(void *ctx) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	Blowfish_Init(&variant->context, variant->key, variant->ksize);
}

/**************************************************************************/
static void blowfish_encrypt_memory__ecb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Encrypt(&variant->context, array, (array+1));
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__ecb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		Blowfish_Decrypt(&variant->context, array, (array+1));
	}
}

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_ECB = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__ecb,
    .decrypt = blowfish_decrypt_memory__ecb
};


/**************************************************************************/
static void blowfish_encrypt_memory__ctr_s(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = ctr ^ *array;
		a_1 = ctr ^ *(array + 1);
		Blowfish_Encrypt(&variant->context, &a_0, &a_1);
		*array = a_0;
		*(array + 1) = a_1;
		ctr++;
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__ctr_s(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = *array;
		a_1 = *(array + 1);
		Blowfish_Decrypt(&variant->context, &a_0, &a_1);
		*array = a_0 ^ ctr;
		*(array + 1) = a_1 ^ ctr;
		ctr++;
	}
}

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_CTR_S = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__ctr_s,
    .decrypt = blowfish_decrypt_memory__ctr_s
};


/**************************************************************************/
static void blowfish_encrypt_memory__ctr_d(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = ctr++ ^ *array;
		a_1 = ctr++ ^ *(array + 1);
		Blowfish_Encrypt(&variant->context, &a_0, &a_1);
		*array = a_0;
		*(array + 1) = a_1;
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__ctr_d(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long ctr = 0;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long *array = (unsigned long *)buffer;	
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {
		a_0 = *array;
		a_1 = *(array + 1);
		Blowfish_Decrypt(&variant->context, &a_0, &a_1);
		*array = a_0 ^ ctr++;
		*(array + 1) = a_1 ^ ctr++;
	}
}

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_CTR_D = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__ctr_d,
    .decrypt = blowfish_decrypt_memory__ctr_d
};


/**************************************************************************/
static void blowfish_encrypt_memory__cbc(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
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

		Blowfish_Encrypt(&variant->context, &a_0, &a_1);

		pa_0 = a_0;
		pa_1 = a_1;

		*array = a_0;
		*(array + 1) = a_1;
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__cbc(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = *array;
		a_1 = *(array + 1);

		Blowfish_Decrypt(&variant->context, &a_0, &a_1);

		a_0 ^= pa_0;
		a_1 ^= pa_1;

		pa_0 = *array;
		pa_1 = *(array+1);

		*array = a_0;
		*(array + 1) = a_1;
	}
}

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_CBC = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__cbc,
    .decrypt = blowfish_decrypt_memory__cbc
};


/**************************************************************************/
static void blowfish_encrypt_memory__cfb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = pa_0;
		a_1 = pa_1;

		Blowfish_Encrypt(&variant->context, &a_0, &a_1);

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;

		pa_0 = a_0;
		pa_1 = a_1;
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__cfb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		Blowfish_Encrypt(&variant->context, &a_0, &a_1);

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

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_CFB = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__cfb,
    .decrypt = blowfish_decrypt_memory__cfb
};


/**************************************************************************/
static void blowfish_encrypt_memory__ofb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		a_0 = pa_0;
		a_1 = pa_1;

		Blowfish_Encrypt(&variant->context, &a_0, &a_1);

		pa_0 = a_0;
		pa_1 = a_1;

		a_0 = a_0 ^ *array;
		a_1 = a_1 ^ *(array + 1);

		*array = a_0;
		*(array + 1) = a_1;
	}
}

/**************************************************************************/
static void blowfish_decrypt_memory__ofb(void *ctx, void* buffer, uint32_t size) 
{
	blowfish_variant_ctx_t *variant = (blowfish_variant_ctx_t *)ctx;
	unsigned long *array = (unsigned long *)buffer;	
	unsigned long a_0 = 0;
	unsigned long a_1 = 0;
	unsigned long pa_0 = 0;
	unsigned long pa_1 = 0;
	for ( ; array+1 < (unsigned long *)(buffer+size+size%2); array += 2) {

		Blowfish_Encrypt(&variant->context, &a_0, &a_1);

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

/**************************************************************************/
static blowfish_variant_ctx_t BLOWFISH_OFB = {
	.init = blowfish_init_ctx,
    .reset = blowfish_reset_ctx,
    .encrypt = blowfish_encrypt_memory__ofb,
    .decrypt = blowfish_decrypt_memory__ofb
};

/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
static blowfish_variant_ctx_t *blowfish_variant = 0;

blowfish_variant_ctx_t *get_blowfish_variant(const char *name)
{
	typedef struct browfish_variants_s {
		const char 				*name;
		blowfish_variant_ctx_t 	*variant;
	} blowfish_variants_t;

	static blowfish_variants_t blowfish_variants[] = {
	#if defined(CONFIG_BLOWFISH_MODE_ECB)
		{ "blowfish_ecb", 	&BLOWFISH_ECB },
	#endif
	#if defined(CONFIG_BLOWFISH_MODE_CTR_S)
		{ "blowfish_ctr_s",	&BLOWFISH_CTR_S },
	#endif
	#if defined(CONFIG_BLOWFISH_MODE_CTR_D)
		{ "blowfish_ctr_d",	&BLOWFISH_CTR_D },
	#endif
	#if defined(CONFIG_BLOWFISH_MODE_CBC)
		{ "blowfish_cbc",	&BLOWFISH_CBC },
	#endif
	#if defined(CONFIG_BLOWFISH_MODE_CFB)
		{ "blowfish_cfb",	&BLOWFISH_CFB },
	#endif
	#if defined(CONFIG_BLOWFISH_MODE_OFB)
		{ "blowfish_ofb",	&BLOWFISH_OFB },
	#endif
		{ 0, 0 }
	};

	blowfish_variants_t *result = blowfish_variants;
    while (result->name) {
		if (!strcmp(name, result->name))
			break;
		result++;
    }
    
    return result->variant;
}

/**************************************************************************/
void blowfish_reset() 
{
	blowfish_variant = get_blowfish_variant(CONFIG_BLOWFISH_VARIANT);

	if (blowfish_variant != 0) {
  		(*blowfish_variant->init)(blowfish_variant,blowfish_key,blowfish_key_size);
  		(*blowfish_variant->reset)(blowfish_variant);
    }
}

/**************************************************************************/
void blowfish_encrypt_memory(void* buffer, uint32_t size) 
{
	if (blowfish_variant != 0) {
		(*blowfish_variant->encrypt)(blowfish_variant,buffer,size);	
	}
}

/**************************************************************************/
void blowfish_decrypt_memory(void* buffer, uint32_t size) 
{
	if (blowfish_variant != 0) {
		(*blowfish_variant->decrypt)(blowfish_variant,buffer,size);
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

void sha1(uint8_t *digest, void *buffer, uint32_t size) {
	SHA_CTX ctx;
	SHAInit(&ctx);
	SHAUpdate(&ctx, buffer, size);
	SHAFinal(digest, &ctx);
}

