#include "stdio.h"
#include <string.h>

/* compiler built-in for va_args */
#include <stdarg.h>
#include <time.h>

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

inline byte_t console_recieved(console_base_p console) {
	if (!console->recieved) return 0;

	return (*console->recieved)(console->ctx);
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

/* Timeout extension */
byte_t waitc(quad_t *tm, byte_t *c) {

	if (!p) return 0;

	quad_t ctm 		= ctime();
	
	quad_t s 		= ctm;
	quad_t delta 	= 0;
	byte_t recv 	= 0;
	while (delta < *tm && !(recv = console_recieved(p)) ) {
		ctm = ctime();
		delta = ctm - s;
		idle();
	};

	if (tm)			*tm -= delta;
	if (c && recv)	*c = console_getc(p);

	return recv;
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

	byte_t flags = 0;
#define FILL_FLAG 0x01
#define LONG_FLAG 0x02

	byte_t c;
	while (c = *fmt++) {

#define OUT_NUMERIC(type,base) \
	{ \
		byte_t wc, ilen; \
		d = va_arg(ap, type); \
		if (flags & FILL_FLAG) { \
			wc = 0; \
			ilen = strilen(d,base); \
			while (wc+ilen < width) { \
				console_putc(console,fill); \
				++wc; \
			} \
		} \
		console_puts(console,itoa(d,base)); \
		flags = 0; \
	}

		if (c == '%' || flags) {
			c = *fmt++;
			if (c == 'l') {
				flags	|=  LONG_FLAG;
				c = *fmt++;
			}

			switch (c)
			{
			case 's':
				{
					s = va_arg(ap, const byte_t *);
					if (s) console_puts(console,s);
					flags = 0;
				}
			break;
			case 'c':
				{
					c = va_arg(ap, int);
					console_putc(console,c);
					flags = 0;
				}
			break;
			/* Fill + Width */
			case 'p':
				{
					if (! (flags & FILL_FLAG) ) {
						flags		|= FILL_FLAG;
						fill		= '0';
						width		= 8;
					}
					console_puts(console,"0x");
				}
			case 'X':
			case 'x':
				{
					if (flags & LONG_FLAG) {
						OUT_NUMERIC(long,16)
					}
					else {
						OUT_NUMERIC(int,16)
					}
				}
			break;
			case 'o':
				{
					if (flags & LONG_FLAG) {
						OUT_NUMERIC(long,8)
					}
					else {
						OUT_NUMERIC(int,8)
					}
				}
			break;
			case 'i':
			case 'd':
				{
					if (flags & LONG_FLAG) {
						OUT_NUMERIC(long,10)
					}
					else {
						OUT_NUMERIC(int,10)
					}
				}
			break;
			default:
				if (flags & FILL_FLAG) {
					console_putc(console,c);
					flags = 0;
				}
				else {
					fill		= c;
					width		= xnumber(*fmt, 16);
					flags		|= FILL_FLAG;
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
