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

void printf(const byte_t *fmt, ...) 
{
	/* Supported %s, %c, %d, %x, %p without fill and width */
	va_list ap;
	va_start(ap,fmt);

	const byte_t *s = 0;
	int d = 0;

	byte_t c;
	while (c = *fmt++) {
		if (c == '%') {
			switch (c = *fmt++)
			{
			case 's':
			{
				s = va_arg(ap, const byte_t *);
				if (s) puts(s);
			}
			break;
			case 'c':
			{
				c = va_arg(ap, int);
				putc(c);
			}
			break;
			case 'd':
			{
				d = va_arg(ap, int);
				puts(itoa(d,10));	
			}
			break;
			case 'p':
			{
				puts("0x");
			}
			case 'X':
			case 'x':
			{
				d = va_arg(ap, int);
				puts(itoa(d,16));	
			}
			break;
			default:
				putc(c);
			}
		}
		else {
			putc(c);
		}
	}

	va_end(ap);
}

