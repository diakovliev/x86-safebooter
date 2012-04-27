#ifndef STDIO_HEADER
#define STDIO_HEADER

#include <loader_types.h>

/* Console interface */
typedef struct console_base_s {
	void *ctx;
	void (*put)(void *ctx, byte_t byte);
	byte_t (*get)(void *ctx);
	byte_t (*recieved)(void *ctx);
} console_base_t, *console_base_p;

/* Service */
void console_init(console_base_p provider);

/* Out */
void putc(byte_t c);
void puts(const byte_t *s);

/* In */
byte_t getc(void);

/* Timeout extension */
byte_t waitc(quad_t *t, byte_t *c);

/* Supported %s, %c, %d, %x, %p without fill and width */
void printf(const byte_t *fmt, ...);
void sprintf(const byte_p dst, const byte_t *fmt, ...);

/* Block input/output stream */

/* Simple abstract interface for working with raw data storages at block level */
typedef struct blk_iostream_s {
	void *ctx;
	word_t (*read)(byte_p dst, word_t size, void *ctx);
	word_t (*write)(byte_p src, word_t size, void *ctx);
	dword_t (*seek)(dword_t pos, void *ctx);
	dword_t (*addr)(void *ctx);
} blk_iostream_t, *blk_iostream_p;

/* Input stream utilites */
static inline word_t blk_read(byte_p dst, word_t count, blk_iostream_p s) {
	if (!s->read) return 0;

	return (*s->read)(dst,count,s->ctx);
}
static inline word_t blk_write(byte_p src, word_t count, blk_iostream_p s) {
	if (!s->write) return 0;

	return (*s->write)(src,count,s->ctx);
}
static inline dword_t blk_seek(dword_t pos, blk_iostream_p s) {
	if (!s->seek) return 0;

	return (*s->seek)(pos,s->ctx);
}
static inline dword_t blk_addr(blk_iostream_p s) {
	if (!s->addr) return 0;

	return (*s->addr)(s->ctx);
}

#endif//STDIO_HEADER

