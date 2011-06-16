#ifndef CRYPT_HEADER
#define CRYPT_HEADER

#include <loader_types.h>

extern void crypt_init(void);
extern void encrypt_memory(void* buffer, dword_t size);
extern void decrypt_memory(void* buffer, dword_t size);

#endif//CRYPT_HEADER

