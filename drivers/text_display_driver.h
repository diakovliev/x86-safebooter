#ifndef TEXT_DISPLAY_DRIVER_HEADER
#define TEXT_DISPLAY_DRIVER_HEADER

#include <loader_types.h>

typedef struct display_s {
	/* pointer to video memory */
	word_t *address;
	/* geometry */
	byte_t width;
	byte_t height;
	/* current cursor position */
	byte_t x;
	byte_t y;
	/* current attribute */
	byte_t attribute;
	/* tab width */
	byte_t tab_width;
} display_t;

#define TXT_VIDEO_MEM	0xB8000

#define TXT_BLACK		0
#define TXT_BLUE		1
#define TXT_GREEN		2
#define TXT_CYAN		3
#define TXT_RED			4
#define TXT_MAGENTA		5
#define TXT_BROWN		6
#define TXT_LGREY		7
#define TXT_DGREY		8
#define TXT_LBLUE		9
#define TXT_LGREEN		10
#define TXT_LCYAN		11
#define TXT_LRED		12
#define TXT_LMAGENTA	13
#define TXT_LBROWN		14
#define TXT_WHITE		15

#define TXT_ATTR(BG,FG) ((BG << 4)|(FG & 0x0F))
#define TXT_DEFAULT		TXT_ATTR(TXT_BLACK,TXT_LGREY)

#define TXT_DEFAULT_TAB_WIDTH	8	

typedef void *display_p;

extern void display_init(display_p d, word_t *address, byte_t width, byte_t height);
extern void display_cursor_move(display_p d, byte_t x, byte_t y);
extern void display_clear(display_p d);
extern void display_out(display_p d, byte_t c, byte_t x, byte_t y);
extern void display_scroll(display_p d);
extern void display_putc(display_p d, byte_t c);
extern void display_puts(display_p d, byte_t *s);

#endif//TXT_DISPLAY_HEADER

