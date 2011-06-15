#ifndef KEYBOARD_DRIVER_HEADER
#define KEYBOARD_DRIVER_HEADER

#include <loader_types.h>

typedef byte_t (*key_handler_t)(byte_t scancode, word_t mod,void *context);
typedef byte_t (*mice_handler_t)(byte_t data, void *context);
typedef byte_t (*idle_handler_t)(void *context);

typedef struct keyboard_driver_s {
	key_handler_t key_handler;
	mice_handler_t mice_handler;
	idle_handler_t idle_handler;
	void *context;
	/* internal data */
	word_t mod;
#define KBD_MOD_ESCAPE_0	(1)
#define KBD_MOD_ESCAPE_1	(1<<1)
#define KBD_MOD_KEY_UP		(1<<2)
#define KBD_MOD_LSHIFT		(1<<3)
#define KBD_MOD_RSHIFT		(1<<4)
#define KBD_MOD_LCTRL		(1<<5)
#define KBD_MOD_RCTRL		(1<<6)
#define KBD_MOD_LALT		(1<<7)
#define KBD_MOD_RALT		(1<<8)
#define KBD_MOD_CAPS		(1<<9)
#define KBD_MOD_NUM			(1<<10)
#define KBD_MOD_SCROLL		(1<<11)
} keyboard_driver_t;
typedef keyboard_driver_t *keyboard_driver_p;

extern byte_t keyboard_init(keyboard_driver_p thiz);
extern byte_t keyboard_run_input_loop(keyboard_driver_p thiz, 
	key_handler_t key_handler, 
	mice_handler_t mice_handler, 
	idle_handler_t idle_handler);

#define KEYBOARD_OK 0x00

#endif//KEYBOARD_DRIVER_HEADER
