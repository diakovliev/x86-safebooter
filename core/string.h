#ifndef STRING_HEADER
#define STRING_HEADER

#include "loader_types.h"

#ifdef TEST_STRING_H
#define STRING_H(x) test_string_h_##x
#else
#define STRING_H(x) x
#endif

/* Base string utils */
extern word_t STRING_H(strlen) (byte_t *s);
extern byte_t STRING_H(strncmp) (byte_t *s0, byte_t *s1, word_t n);
extern byte_t STRING_H(strcmp) (byte_t *s0, byte_t *s1);
extern byte_t STRING_H(starts_from) (byte_t *s0, byte_t *s1);
extern word_t STRING_H(memcpy) (void *dst, void *src, word_t sz);
extern word_t STRING_H(memset) (void *dst, byte_t src, word_t sz);
extern word_t STRING_H(strcpy) (byte_t *dst, byte_t *src);
extern byte_t *STRING_H(strtok) (byte_t *sep, byte_t *str);
extern byte_t *STRING_H(strltrim) (byte_t *trim, byte_t *str);
extern void STRING_H(strrev) (byte_t *s, word_t ln);
extern char STRING_H(xnumber) (byte_t ch, byte_t base);
extern unsigned long STRING_H(atol) (byte_t *str, byte_t base);
extern byte_t *STRING_H(itoa) (long value, byte_t base);

#endif/*STRING_HEADER*/

