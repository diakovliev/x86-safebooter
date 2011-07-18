#include "console_iface.h"
#include <string.h>

/* compiler built-in for va_args */
#include <stdarg.h>

static console_base_p p = 0;

/* Service */
void console_init(console_base_p provider) {
	p = provider;
}

/* Out */
void putc(byte_t c) {
	if (p && p->put) (*p->put)(p->ctx,c);
}

void puts(const byte_t *s) {
	if (!p) return;

	byte_t c;
	while (c = *s++) {
		putc(c);
	}
}

/* In */
byte_t getc(void) {
	if (!p || !p->get) return 0;

	return (*p->get)(p->ctx);
}

inline byte_t strilen(long value, byte_t base) {
	return strlen(itoa(value,base));
}

#define OUT_NUMERIC(type,base) \
	d = va_arg(ap, type); \
	if (fill_flag) { \
		wc = 0; \
		while (wc+strilen(d,base) < width) { \
			putc(fill); \
			++wc; \
		} \
	} \
	puts(itoa(d,base));

void printf(const byte_t *fmt, ...) 
{
	/* Supported %s, %c, %d, %x, %p with fill and width */
	va_list ap;
	va_start(ap,fmt);

	const byte_t *s = 0;
	int d = 0;

	byte_t fill = ' ';
	byte_t width = 0;
	byte_t fill_flag = 0;
	byte_t wc;

	byte_t c;
	while (c = *fmt++) {
		if (c == '%' || fill_flag) {
			switch (c = *fmt++)
			{
			case 's':
				{
					s = va_arg(ap, const byte_t *);
					if (s) puts(s);
					fill_flag	= 0;
				}
			break;
			case 'c':
				{
					c = va_arg(ap, int);
					putc(c);
					fill_flag	= 0;
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
					puts("0x");
				}
			case 'X':
			case 'x':
				{
					OUT_NUMERIC(int,16)
					fill_flag	= 0;
				}
			break;
			case 'd':
				{
					OUT_NUMERIC(int,10)
					fill_flag	= 0;
				}
			break;
			default:
				if (!fill_flag) {
					fill		= c;
					width		= xnumber(*fmt, 16);
					fill_flag	= 1;
				}
				else {
					putc(c);
					fill_flag	= 0;
				}
			}
		}
		else {
			putc(c);
		}
	}

	va_end(ap);
}

