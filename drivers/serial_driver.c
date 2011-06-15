#include "serial_driver.h"
#include <loader_types.h>
#include <common.h>

/* Routes */
void ser_set_divisor(word_t port, word_t div) {
	byte_t lcr = inb(SER_LCR_PORT(port));
	outb(SER_LCR_PORT(port), SER_DLAB|lcr);
	outb(port+0, div);
	outb(port+1, div>>8);
	lcr &= ~SER_DLAB; 
	outb(SER_LCR_PORT(port), lcr);
}

void ser_init(word_t port) {
	/* Disable interrupts */
	outb(SER_INT_ID_PORT(port), 0);
	/* Set communication speed to 115200 */
	ser_set_divisor(port, 1);
	/* Data bits | Stop bits | Parity */
	outb(SER_LCR_PORT(port), SER_DATA_BITS_8|SER_STOP_BITS_1|SER_PARITY_NONE);
}

byte_t ser_received(word_t port) {
	return inb(SER_LSR_PORT(port)) & 1;
}

byte_t ser_read(word_t port) {
	while (!ser_received(port)) idle();

	return inb(SER_DATA_PORT(port));
}

byte_t ser_is_transmit_empty(word_t port) {
	return inb(SER_LSR_PORT(port)) & 0x20;
}

void ser_write(word_t port, byte_t byte) {
	while (!ser_is_transmit_empty(port)) idle();

	outb(SER_DATA_PORT(port), byte);
}

void ser_write_string(word_t port, const byte_t *str) {
	byte_t c;
	while (c = *str++) {
		ser_write(port,c);
	}
}

/* Console intefrace */
struct ser_console_context_s {
	word_t port;
};

void ser_console_put(void *ctx, byte_t byte) {
	if (!ctx) return;
	
	struct ser_console_context_s *sctx = (struct ser_console_context_s *)ctx;

	ser_write(sctx->port,byte);	
}

byte_t ser_console_get(void *ctx) {
	if (!ctx) return 0;
	
	struct ser_console_context_s *sctx = (struct ser_console_context_s *)ctx;

	return ser_read(sctx->port);
}

/* Static serial console data */
static struct ser_console_context_s ser_console_context = {
	.port = 0
};

static console_base_t ser_console = {
	.ctx = &ser_console_context,
	.put = ser_console_put,
	.get = ser_console_get
};

/* */
console_base_p ser_get_console(word_t port) {
	ser_console_context.port = port;
	return &ser_console;
}

