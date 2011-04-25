#include "text_display_driver.h"
#include <common.h>

#define D(x) ((display_t*)x)

void display_init(display_p d, word_t *address, byte_t width, byte_t height) {
	D(d)->address		= address;
	D(d)->width			= width;
	D(d)->height		= height;
	D(d)->attribute		= TXT_DEFAULT;
	D(d)->x = D(d)->y	= 0;
	D(d)->tab_width		= TXT_DEFAULT_TAB_WIDTH;
} 

void display_cursor_move(display_p d, byte_t x, byte_t y) {
	word_t location = (y*D(d)->width+x) + (D(d)->address-(word_t*)TXT_VIDEO_MEM);
	outb(0x3d4,14);
	outb(0x3d5,location>>8);
	outb(0x3d4,15);
	outb(0x3d5,location);
	D(d)->x = x;
	D(d)->y = y;
}

void display_clear(display_p d) {
	int i;
	word_t blank = TXT_DEFAULT<<8|' ';
	for (i = 0; i < D(d)->width*D(d)->height; ++i) {
		D(d)->address[i] = blank;
	}
	D(d)->x = D(d)->y	= 0;
}

void display_out(display_p d, byte_t c, byte_t x, byte_t y) {
	D(d)->address[y*D(d)->width+x] = c|D(d)->attribute<<8;
}

void display_scroll(display_p d) {
	word_t blank = TXT_DEFAULT<<8|' ';
	if (D(d)->y >= D(d)->height) {
    	int i;
		for (i = 0; i < (D(d)->height-1)*D(d)->width; ++i)
			D(d)->address[i] = D(d)->address[i+D(d)->width];
		
		for (i = (D(d)->height-1)*D(d)->width; i < D(d)->height*D(d)->width; ++i)
			D(d)->address[i] = blank;

		D(d)->y = D(d)->height-1;
	}
}

void display_putc(display_p d, byte_t c) {
	switch (c) {
	case '\r': {
			D(d)->y = D(d)->y;
			D(d)->x = 0;
		}
		break;
	case '\n': {
			D(d)->x = D(d)->x;
			D(d)->y += 1;
		}
		break;
	case '\t': {
			D(d)->x = (D(d)->x+D(d)->tab_width)&~(D(d)->tab_width-1);
			D(d)->y = D(d)->y;
		}
		break;
	default: {
			display_out(d,c,D(d)->x,D(d)->y);
			D(d)->y = D(d)->x+1 > D(d)->width ? D(d)->y+1 : D(d)->y;
			D(d)->x = D(d)->x+1 > D(d)->width ? 0 : D(d)->x+1;
		}
	}
	display_scroll(d);
	display_cursor_move(d,D(d)->x,D(d)->y);
}

void display_puts(display_p d, byte_t *s) {
	while (*s) {
		display_putc(d,*s++);
	}
}

