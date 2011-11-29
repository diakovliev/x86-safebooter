#include <common.h>
#include "text_display_console.h"
#include "ascii_driver.h"

#include <stdio.h>

struct text_display_console_context_s {
	display_p display;
	keyboard_driver_p keyboard;
	byte_t c;
	word_t skip;
	byte_t recieved;
};

static struct text_display_console_context_s text_display_console_context = {
	.display = 0,
	.keyboard = 0,
	.c = 0,
	.skip = 0,
	.recieved = 0,
};

static byte_t key_handler(byte_t scancode, word_t mod, void *ctx)
{
	ctx = ctx;
	
	text_display_console_context.skip = !(mod & KBD_MOD_KEY_UP);
	text_display_console_context.c 	  = ascii_2ascii(scancode, mod);
	if (!text_display_console_context.skip) {
		text_display_console_context.recieved	= 1;
	}

	return 1; 
}

static byte_t idle_handler(void *ctx)
{
	ctx = ctx;

	return 1; 
}

static void text_display_console_put(void *ctx, byte_t byte) {
	if (!ctx) return;

	struct text_display_console_context_s *context = 
		(struct text_display_console_context_s *)ctx;

	display_putc(context->display, byte);
}

static byte_t text_display_console_recieved(void *ctx) {
	if (!ctx) return 0;
	
	struct text_display_console_context_s *context = 
		(struct text_display_console_context_s *)ctx;

	if (!context->recieved) {
		keyboard_run_input_loop(context->keyboard, key_handler, 0, idle_handler);
	}

	return context->recieved;
}

static byte_t text_display_console_get(void *ctx) {
	if (!ctx) return 0;
	
	struct text_display_console_context_s *context = 
		(struct text_display_console_context_s *)ctx;

	while (!context->recieved) {
		keyboard_run_input_loop(context->keyboard, key_handler, 0, 0);
	}

	context->recieved = 0;
	return context->c;
}

static console_base_t text_display_console = {
	.ctx = &text_display_console_context,
	.put = text_display_console_put,
	.get = text_display_console_get,
	.recieved = text_display_console_recieved,
};

console_base_p display_get_console(display_p display, keyboard_driver_p keyboard)
{
	text_display_console_context.display = display;
	text_display_console_context.keyboard = keyboard;
	
	return &text_display_console;
}
