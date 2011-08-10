#ifndef STDIO_HEADER
#define STDIO_HEADER

#include <loader_types.h>

/* Console interface */
typedef struct console_base_s {
	void *ctx;
	void (*put)(void *ctx, byte_t byte);
	byte_t (*get)(void *ctx);
} console_base_t, *console_base_p;

/* Service */
void console_init(console_base_p provider);

/* Out */
void putc(byte_t c);
void puts(const byte_t *s);

/* In */
byte_t getc(void);

/* Supported %s, %c, %d, %x, %p without fill and width */
void printf(const byte_t *fmt, ...);
void sprintf(const byte_p dst, const byte_t *fmt, ...);

/* Block input stream */

/* Simple abstract interface for working with raw data storages */
typedef struct blk_istream_s {
	void *ctx;
	word_t (*read)(byte_p dst, word_t size, void *ctx);
	dword_t (*seek)(dword_t pos, void *ctx);
	dword_t (*addr)(void *ctx);
} blk_istream_t, *blk_istream_p;

/* Input stream utilites */
static inline word_t bs_read(byte_p dst, word_t count, blk_istream_p s) {
	return (*s->read)(dst,count,s->ctx);
}
static inline dword_t bs_seek(dword_t pos, blk_istream_p s) {
	return (*s->seek)(pos,s->ctx);
}
static inline dword_t bs_addr(blk_istream_p s) {
	return (*s->addr)(s->ctx);
}

#endif//STDIO_HEADER
