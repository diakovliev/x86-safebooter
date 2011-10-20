#ifndef STRING_HEADER
#define STRING_HEADER

#include "loader_types.h"

/* Base string utils */
word_t strlen(const byte_p s);
byte_t strncmp(byte_t *s0, byte_t *s1, word_t n);
byte_t strcmp(byte_t *s0, byte_t *s1);
byte_t starts_from(byte_t *s0, byte_t *s1);
word_t memcpy(void *dst, void *src, word_t sz);
word_t memset(void *dst, byte_t src, word_t sz);
word_t strcpy(byte_t *dst, byte_t *src);
byte_t *strtok(byte_t *sep, byte_t *str);
byte_t *strltrim(byte_t *trim, byte_t *str);
void strrev(byte_t *s, word_t ln);
char xnumber(byte_t ch, byte_t base);
unsigned long atol(byte_t *str, byte_t base);
byte_t *itoa(long value, byte_t base);

#endif/*STRING_HEADER*/

