#include "string.h"

/* Base string utils */

word_t strlen(const byte_p s) {
	word_t r = 0;
	byte_p w = (byte_p)s;
	while( *w++ ) ++r;
	return r;
}

byte_t strncmp(byte_t *s0, byte_t *s1, word_t n) {
	byte_t r = 0;
	word_t cnt = 0;
	while ( (*s0 || *s1) && !(r = *s0-*s1) && cnt < n-1 ) {
		++cnt, ++s0, ++s1;
	}
	return r;
}

byte_t strcmp(byte_t *s0, byte_t *s1) {
	byte_t r = 0;
	while ( (*s0 || *s1) && !(r = *s0-*s1) ) {
		++s0, ++s1;
	}
	return r;
}

byte_t starts_from(byte_t *s0, byte_t *s1) {
	byte_t r = 0;
	while ( *s0 && *s1 && !(r = *s0-*s1) ) {
		++s0, ++s1;
	}
	return (!*s1 && !r) ? 1 : 0;
}

word_t memcpy(void *dst, void *src, word_t sz) {
	word_t cnt = 0;
	while(cnt < sz) {
		((byte_t*)dst)[cnt] = ((byte_t*)src)[cnt++];
	}
	return cnt;
}

word_t memset(void *dst, byte_t src, word_t sz) {
	word_t cnt = 0;
	while(cnt < sz) {
		((byte_t*)dst)[cnt++] = src;
	}
	return cnt;
}

word_t strcpy(byte_t *dst, byte_t *src) {
	word_t sz = strlen(src) + 1;
	word_t res = memcpy(dst,src,sz);
	dst[sz] = 0;
	return res;
}

byte_t *strtok(byte_t *sep, byte_t *str) {
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


byte_t *strltrim(byte_t *trim, byte_t *str) {
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
void strrev(byte_t *s, word_t ln) {
	word_t i;
	byte_t c;
	for (i = 0; i < ln/2; i++) {
		c = s[ln-1-i];
		s[ln-1-i] = s[i];
		s[i] = c;
	}
}

char xnumber(byte_t ch, byte_t base) {
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

unsigned long atol(byte_t *str, byte_t base) {

	unsigned long r = 0;

	while (xnumber(*str,base) >= 0) {
		r += xnumber(*str++,base);
		r *= base;
	}

	return r/base;
}

byte_t *itoa(long value, byte_t base) {
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
	strrev(res, buf-res-1);
	return res;
}
