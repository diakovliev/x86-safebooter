#include "keyboard_driver.h"
#include <common.h>

byte_t keyboard_init(keyboard_driver_p thiz, void *context)
{
	thiz->context = context;
	thiz->mod = 0;
	return KEYBOARD_OK;	
}


byte_t keyboard_driver_key_handler(keyboard_driver_p thiz, byte_t data)
{
	byte_t scancode = data;
	byte_t res = KEYBOARD_OK;
	
	switch (scancode) {
	/* escape code */
	case 0xe0:
		thiz->mod |= KBD_MOD_ESCAPE_0;
		return res;
	case 0xe1: 
		thiz->mod |= KBD_MOD_ESCAPE_1;
		return res;
	/* keyboard error */
	case 0x00:
	case 0xff:
		return res;
	}

	/* key up/down */	
	if (scancode & 0x80) {
		scancode &= ~0x80;
		thiz->mod |= KBD_MOD_KEY_UP;
	}
	else
	{
		thiz->mod &= ~KBD_MOD_KEY_UP;
	}

#define MOD_PRESS_ESCAPED(KEY,MODIFIER,ESCAPE_0_MODIFIER) \
	case KEY: \
	if (thiz->mod & KBD_MOD_KEY_UP) { \
		thiz->mod &=~ thiz->mod&KBD_MOD_ESCAPE_0?ESCAPE_0_MODIFIER:MODIFIER; \
	} \
	else { \
		thiz->mod |= thiz->mod&KBD_MOD_ESCAPE_0?ESCAPE_0_MODIFIER:MODIFIER; \
	} \
	thiz->mod &= ~KBD_MOD_ESCAPE_0; \
	thiz->mod &= ~KBD_MOD_ESCAPE_1; \
	return res;

#define MOD_PRESS(KEY,MODIFIER) \
	case KEY: \
	if (thiz->mod & KBD_MOD_KEY_UP) { \
		thiz->mod &=~ MODIFIER; \
	} \
	else { \
		thiz->mod |= MODIFIER; \
	} \
	return res;

#define REVERT_BIT(X,BIT) X = (X & !BIT)|(X ^ BIT)

#define MOD(KEY,MODIFIER) \
	case KEY: \
	if (!(thiz->mod & KBD_MOD_KEY_UP)) { \
		REVERT_BIT(thiz->mod,MODIFIER); \
	} \
	return res;

	switch (scancode) {
	/* modifiers */
	MOD_PRESS_ESCAPED(0x1d,KBD_MOD_LCTRL,KBD_MOD_RCTRL);
	MOD_PRESS_ESCAPED(0x38,KBD_MOD_LALT,KBD_MOD_RALT);
	MOD_PRESS(0x2a,KBD_MOD_LSHIFT);
	MOD_PRESS(0x36,KBD_MOD_RSHIFT);
	MOD(0x3a,KBD_MOD_CAPS);
	MOD(0x45,KBD_MOD_NUM);
	MOD(0x46,KBD_MOD_SCROLL);
	}

#undef MOD_PRESS_ESCAPED
#undef MOD_PRESS
#undef MOD

// TODO: Process leds 

	/* call user handler */
	if (thiz->key_handler) {
		res = (*thiz->key_handler) (scancode,thiz->mod,thiz->context);
	}
	
	/* reset escape */
	thiz->mod &= ~KBD_MOD_ESCAPE_0;	
	thiz->mod &= ~KBD_MOD_ESCAPE_1;

	return res;
}

byte_t keyboard_driver_mice_handler(keyboard_driver_p thiz, byte_t scancode)
{
	byte_t res = KEYBOARD_OK;
	if (thiz->mice_handler)
		res = (*thiz->mice_handler) (scancode,thiz->context);
	return res;
}

#define KBD_DATA_IN_BUFFER	(1)
#define KBD_BUFFER_IS_EMPTY	(1<<2)
#define KBD_MICE_DATA		(1<<5)
#define KBD_TIMEOUT			(1<<6)
#define KBD_PARITY_ERROR	(1<<7)

byte_t keyboard_run_input_loop(keyboard_driver_p thiz, 
	key_handler_t key_handler, 
	mice_handler_t mice_handler, 
	idle_handler_t idle_handler)
{
	thiz->key_handler = key_handler;
	thiz->mice_handler = mice_handler;
	thiz->idle_handler = idle_handler;
	
	byte_t res 		= KEYBOARD_OK;
	byte_t status	= 0;
	byte_t data		= 0;
	byte_t modifiers= 0;

	do {
		status = inb(0x64);

		if (KBD_PARITY_ERROR & status) {
			idle();
			continue;
		}

		if (KBD_DATA_IN_BUFFER & status) {
			data = inb(0x60);
			if (KBD_MICE_DATA & status) {
				res = keyboard_driver_mice_handler(thiz,data);
			}
			else {
				res = keyboard_driver_key_handler(thiz,data);
			}
		}
		else
		if (thiz->idle_handler) {
			res = (*thiz->idle_handler) (thiz->context);
		}
		else {
			idle();
		}

	}
	while (res == KEYBOARD_OK);

	return res;
}
