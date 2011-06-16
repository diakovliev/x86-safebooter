#include "text_display_console.h"
#include "ascii_driver.h"

struct text_display_console_context_s {
	display_p display;
	keyboard_driver_p keyboard;
	byte_t c;
	word_t skip;
};

static struct text_display_console_context_s text_display_console_context = {
	.display = 0,
	.keyboard = 0,
	.c = 0,
	.skip = 0
};


static void text_diaplay_console_put(void *ctx, byte_t byte) {
	if (!ctx) return;

	struct text_display_console_context_s *context = 
		(struct text_display_console_context_s *)ctx;

	display_putc(context->display, byte);
}

static byte_t key_handler(byte_t scancode, word_t mod, void *ctx)
{
	ctx = ctx;
	
	text_display_console_context.c 		= ascii_2ascii(scancode, mod);
	text_display_console_context.skip	= !(mod & KBD_MOD_KEY_UP);

	return 1; 
}

static byte_t text_display_console_get(void *ctx) {
	if (!ctx) return 0;
	
	struct text_display_console_context_s *context = 
		(struct text_display_console_context_s *)ctx;

	do {
		keyboard_run_input_loop(context->keyboard, key_handler, 0, 0);
	} while (!context->skip);
	
	return context->c;
}

static console_base_t text_display_console = {
	.ctx = &text_display_console_context,
	.put = text_diaplay_console_put,
	.get = text_display_console_get
};

console_base_p display_get_console(display_p display, keyboard_driver_p keyboard)
{
	text_display_console_context.display = display;
	text_display_console_context.keyboard = keyboard;
	
	return &text_display_console;
}
