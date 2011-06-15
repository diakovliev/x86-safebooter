#ifndef SERIAL_DRIVER_HEADER
#define SERIAL_DRIVER_HEADER

#include <loader_types.h>
#include "console_iface.h"

/* Ports */
#define COM1	0x3F8
#define COM2	0x2F8
#define COM3	0x3E8
#define COM4	0x2E8

/* Port mapping */
#define SER_DATA_PORT(x)	(x)
#define SER_INT_PORT(x)		(x+1)
#define SER_INT_ID_PORT(x)	(x+2)
#define SER_LCR_PORT(x)		(x+3)
#define SER_MCR_PORT(x)		(x+4)
#define SER_LSR_PORT(x)		(x+5)
#define SER_MSR_PORT(x)		(x+6)
#define SER_SCR_PORT(x)		(x+7)

/* DLAB */
#define SER_DLAB			0x80

/* Data bits */
#define SER_DATA_BITS_5		0
#define SER_DATA_BITS_6		1
#define SER_DATA_BITS_7		2
#define SER_DATA_BITS_8		3

/* Stop bits */
#define SER_STOP_BITS_1		0
#define SER_STOP_BITS_2		4

/* Parity */
#define SER_PARITY_NONE		0
#define SER_PARITY_ODD		8
#define SER_PARITY_EVEN		24
#define SER_PARITY_MARK		40
#define SER_PARITY_SPACE	56

/* Routes */
extern void ser_set_divisor(word_t port, word_t div);
extern void ser_init(word_t port);
extern byte_t ser_received(word_t port);
extern byte_t ser_read(word_t port);
extern byte_t ser_is_transmit_empty(word_t port);
extern void ser_write(word_t port, byte_t byte);
extern void ser_write_string(word_t port, const byte_t *str);

extern console_base_p ser_get_console(word_t port);

#endif//SERIAL_DRIVER_HEADER

