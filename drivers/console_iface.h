#ifndef CONSOLE_IFACE_HEADER
#define CONSOLE_IFACE_HEADER

#include <loader_types.h>

typedef struct console_base_s {
	void *ctx;
	void (*put)(void *ctx, byte_t byte);
	byte_t (*get)(void *ctx);
} console_base_t, *console_base_p;

/* Service */
extern void console_init(console_base_p provider);

/* Out */
extern void putc(byte_t c);
extern void puts(const byte_t *s);

/* In */
extern byte_t getc(void);

#endif//CONSOLE_IFACE_HEADER
