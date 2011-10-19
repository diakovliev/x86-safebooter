#include "stdio.h"
#include <string.h>

/* compiler built-in for va_args */
#include <stdarg.h>

static console_base_p p = 0;

/* ----------------------------------------------------------- */
/* Service */
void console_init(console_base_p provider) {
	p = provider;
}

inline void console_putc(console_base_p console, const byte_t c) {
	if (!console->put) return;

	(*console->put)(console->ctx, c);
}

inline void console_puts(console_base_p console, const byte_t *s) {
	byte_t c;
	while (c = *s++) {
		console_putc(console, c);
	}
}

inline byte_t console_getc(console_base_p console) {
	if (!console->get) return 0;

	return (*console->get)(console->ctx);
}

/* ----------------------------------------------------------- */
/* Out */
void putc(byte_t c) {
	if (!p) return;

	console_putc(p, c);
}

void puts(const byte_t *s) {
	if (!p) return;

	console_puts(p, s);
}

/* In */
byte_t getc(void) {
	if (!p) return 0;

	return console_getc(p);
}

/* ----------------------------------------------------------- */
/* Memory */
struct mem_context_s {
	byte_p base_ptr;
	byte_p current_ptr;
};

inline void mem_putc(void *ctx, byte_t c) {
	struct mem_context_s *mem_ctx = (struct mem_context_s *)ctx;
	*mem_ctx->current_ptr = c;
	++mem_ctx->current_ptr;
}

console_base_p mem_init(const byte_p ptr) {
	static console_base_t console;
	static struct mem_context_s mem_ctx;
	mem_ctx.base_ptr = ptr;
	mem_ctx.current_ptr = ptr;
	console.ctx = &mem_ctx;
	console.put = mem_putc;
	return &console;
}

/* ----------------------------------------------------------- */
inline byte_t strilen(long value, byte_t base) {
	return strlen(itoa(value,base));
}


void va_printf(console_base_p console, const byte_t *fmt, va_list ap)
{
	/* Supported %s, %c, %o, %i, %d, %x, %X, %p, %lo, %li, %ld, %lx, %lX, %pwith fill and width */


	const byte_t *s = 0;
	long d = 0;

	byte_t fill = ' ';
	byte_t width = 0;
	byte_t fill_flag = 0;
	byte_t long_flag = 0;
	byte_t wc, ilen;

	byte_t c;
	while (c = *fmt++) {

#define OUT_NUMERIC(type,base) \
	{ \
		d = va_arg(ap, type); \
		if (fill_flag) { \
			wc = 0; \
			ilen = strilen(d,base); \
			while (wc+ilen < width) { \
				console_putc(console,fill); \
				++wc; \
			} \
		} \
		console_puts(console,itoa(d,base)); \
		long_flag = 0; \
		fill_flag = 0; \
	}

		if (c == '%' || fill_flag) {
			c = *fmt++;
			if (c == 'l') {
				long_flag	= 1;
				c = *fmt++;
			}

			switch (c)
			{
			case 's':
				{
					s = va_arg(ap, const byte_t *);
					if (s) console_puts(console,s);
					fill_flag	= 0;
					long_flag	= 0;
				}
			break;
			case 'c':
				{
					c = va_arg(ap, int);
					console_putc(console,c);
					fill_flag	= 0;
					long_flag	= 0;
				}
			break;
			/* Fill + Width */
			case 'p':
				{
					if (!fill_flag) {
						fill_flag	= 1;
						fill		= '0';
						width		= 8;
					}
					console_puts(console,"0x");
				}
			case 'X':
			case 'x':
				{
					if (!long_flag) {
						OUT_NUMERIC(int,16)
					}
					else {
						OUT_NUMERIC(long,16)
					}
				}
			break;
			case 'o':
				{
					if (!long_flag) {
						OUT_NUMERIC(int,8)
					}
					else {
						OUT_NUMERIC(long,8)
					}
				}
			break;
			case 'i':
			case 'd':
				{
					if (!long_flag) {
						OUT_NUMERIC(int,10)
					}
					else {
						OUT_NUMERIC(long,10)
					}
				}
			break;
			default:
				if (!fill_flag) {
					fill		= c;
					width		= xnumber(*fmt, 16);
					fill_flag	= 1;
				}
				else {
					console_putc(console,c);
					fill_flag	= 0;
					long_flag	= 0;
				}
			}
		}
		else {
			console_putc(console,c);
		}
	}
}

void printf(const byte_t *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	va_printf(p,fmt,ap);
	va_end(ap);
}

void sprintf(const byte_p dst, const byte_t *fmt, ...) {
	va_list ap;
	va_start(ap,fmt);
	va_printf(mem_init(dst),fmt,ap);
	va_end(ap);
}
