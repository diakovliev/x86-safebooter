#include "console_iface.h"

static console_base_p p = 0;

/* Service */
void console_init(console_base_p provider) {
	p = provider;
}

/* Out */
void putc(byte_t c) {
	if (p) (*p->put)(p->ctx,c);
}

void puts(const byte_t *s) {
	byte_t c;
	while (c = *s++) {
		putc(c);
	}
}

/* In */
byte_t getc(void) {
	if (!p) return 0;

	return (*p->get)(p->ctx);
}

