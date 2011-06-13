#ifndef STRING_HEADER
#define STRING_HEADER

#include "loader_types.h"

#ifdef TEST_STRING_H
#define STRING_H(x) test_string_h_##x
#else
#define STRING_H(x) x
#endif

/* Base string utils */
static inline word_t STRING_H(strlen) (byte_t *s) {
	word_t r = 0;
	while( *s++ ) ++r;
	return r;
}

static inline byte_t STRING_H(strncmp) (byte_t *s0, byte_t *s1, word_t n) {
	byte_t r = 0;
	word_t cnt = 0;
	while ( *s0 || *s1 && !(r = *s0-*s1) && cnt < n-1 ) {
		++cnt, ++s0, ++s1;
	}
	return r;
}

static inline byte_t STRING_H(strcmp) (byte_t *s0, byte_t *s1) {
	byte_t r = 0;
	while ( *s0 || *s1 && !(r = *s0-*s1) ) {
		++s0, ++s1;
	}
	return r;
}

static inline byte_t STRING_H(starts_from) (byte_t *s0, byte_t *s1) {
	byte_t r = 0;
	while ( *s0 && *s1 && !(r = *s0-*s1) ) {
		++s0, ++s1;
	}
	return !r;
}

static inline word_t STRING_H(memcpy) (void *dst, void *src, word_t sz) {
	word_t cnt = 0;
	while(cnt < sz) {
		((byte_t*)dst)[cnt] = ((byte_t*)src)[cnt++];
	}
	return cnt;
}

static inline word_t STRING_H(memset) (void *dst, byte_t src, word_t sz) {
	word_t cnt = 0;
	while(cnt < sz-1) {
		((byte_t*)dst)[cnt++] = src;
	}
	return cnt;
}


static inline word_t STRING_H(strcpy) (byte_t *dst, byte_t *src) {
	word_t sz = STRING_H(strlen) (src) + 1;
	return STRING_H(memcpy) (dst,src,sz);
}

/* !!! Modify buffer str !!! */
static inline byte_t *STRING_H(strtok) (byte_t *sep, byte_t *str) {
	static byte_t *buf = 0;
	if (str) {
		byte_t *old_buf = buf;
		buf = str;
		return old_buf;
	}
	if (!*buf) {
		return 0;
	}
	byte_t *res = buf;
	while (*buf) {
		byte_t *sp = sep;
		while (*sp) {
			if (*sp == *buf) {
				*buf = 0;
				++buf;
				return res; 
			}
			++sp;
		}
		++buf;
	}
	return res;
}


static inline byte_t *STRING_H(strltrim) (byte_t *trim, byte_t *str) {
	byte_t *res = str;
	byte_t *trim_p = trim;

	while (*trim_p && *res) {
		if (*res == *trim_p) {
			res++;
			trim_p = trim;
		}
		else {
			++trim_p;
		}
	}

	return res;
}

/* !!! Modify buffer s !!! */
static inline void STRING_H(strrev) (byte_t *s, word_t ln) {
	word_t i;
	byte_t c;
	for (i = 0; i < ln/2; i++) {
		c = s[ln-1-i];
		s[ln-1-i] = s[i];
		s[i] = c;
	}
}

static inline char STRING_H(xnumber) (byte_t ch, byte_t base) {
	if (ch >= 0x30 && ch <= 0x39) /* 0..9 */
		return (ch - 0x30);
	else
	if (ch >= 0x41 && ch <= 0x46 && base > 10) /* A..F */
		return (ch - 0x37);
	else
	if (ch >= 0x61 && ch <= 0x66 && base > 10) /* a..f */
		return (ch - 0x57);
	else
		return -1;
}

static unsigned long STRING_H(atol) (byte_t *str, byte_t base) {

	unsigned long r = 0;	

	while (STRING_H(xnumber) (*str,base) >= 0) {
		r += STRING_H(xnumber) (*str++,base);
		r *= base;
	}
		
	return r/base;
}

static inline byte_t *STRING_H(itoa) (unsigned long value, byte_t base) {
	/* max is 128-bit binary value with sign */
	static byte_t res[129];
	byte_t *buf = res;
	long v = value>=0?value:-value;
	byte_t ch = '0';
	byte_t n;
	do {
		n = v % base;
		v /= base;
		if (n < 10)
			ch = n + 0x30;
		else
			ch = n + 0x37;
		*buf++ = ch;
	} while (v);
	*buf++ = 0;
	STRING_H(strrev) (res, buf-res-1);
	return res;
}

#endif/*STRING_HEADER*/

