//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include "loader_types.h"

static void outb(word_t port, byte_t byte) {
	asm volatile ("outb %1,%0" : : "dN" (port), "a"(byte) );
}

static byte_t inb(word_t port) {
   byte_t ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

static word_t inw(word_t port) {
   word_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

typedef struct display_s {
	/* pointer to video memory */
	word_t *address;
#define TXT_VIDEO_MEM	0xB8000
	/* geometry */
	byte_t width;
	byte_t height;
	/* current cursor position */
	byte_t x;
	byte_t y;
	/* current attribute */
	byte_t attribute;
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
	/* tab width */
	byte_t tab_width;
#define TXT_DEFAULT_TAB_WIDTH	8	
} display_t;

void display_init(display_t *d, word_t *address, byte_t width, byte_t height) {
	d->address		= address;
	d->width		= width;
	d->height		= height;
	d->attribute	= TXT_DEFAULT;
	//display_cursor_move(d,0,0);
	d->x = d->y		= 0;
	d->tab_width	= TXT_DEFAULT_TAB_WIDTH;
} 

void display_cursor_move(display_t *d, byte_t x, byte_t y) {
	word_t location = (y*d->width+x) + (d->address-(word_t*)TXT_VIDEO_MEM);
	outb(0x3d4,14);
	outb(0x3d5,location>>8);
	outb(0x3d4,15);
	outb(0x3d5,location);
	d->x = x;
	d->y = y;
}

void display_clear(display_t *d) {
	int i;
	word_t blank = TXT_DEFAULT<<8|' ';
	for (i = 0; i < d->width*d->height; ++i) {
		d->address[i] = blank;
	}
}

void display_out(display_t *d, byte_t c, byte_t x, byte_t y) {
	d->address[y*d->width+x] = c|d->attribute<<8;
}

void display_scroll(display_t *d) {
	word_t blank = TXT_DEFAULT<<8|' ';
	if (d->y >= d->height) {
    	int i;
		for (i = d->width; i < (d->height-1)*d->width; ++i)
			d->address[i] = d->address[i+d->width];
		
		for (i = (d->height-1)*d->width; i < d->height*d->width; ++i)
			d->address[i] = blank;

		d->y = d->height-1;
	}
}

void display_putc(display_t *d, byte_t c) {
	switch (c) {
	case '\r': {
			d->y = d->y;
			d->x = 0;
		}
		break;
	case '\n': {
			d->x = d->x;
			d->y += 1;
		}
		break;
	case '\t': {
			d->x = (d->x+d->tab_width)&~(d->tab_width-1);
			d->y = d->y;
		}
		break;
	default: {
			display_out(d,c,d->x,d->y);
			d->y = d->x+1 > d->width ? d->y+1 : d->y;
			d->x = d->x+1 > d->width ? 0 : d->x+1;
		}
	}
	//display_scroll(d);
	display_cursor_move(d,d->x,d->y);
}

void display_puts(display_t *d, byte_t *s) {
	while (*s) {
		display_putc(d,*s++);
	}
}


/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	display_t d0;
	display_t d1;
	
	display_init(&d0, TXT_VIDEO_MEM + 80*2*10, 80, 15);
	display_clear(&d0);
	
	display_init(&d1, TXT_VIDEO_MEM, 80, 10);
	display_clear(&d1);
	
	int i;
	for (i = 0; i < 1500; ++i) {
		if (i%2) 
			display_puts(&d0,"Hello\r\n\tWorld");
		else
			display_puts(&d1,"\r\n1234567\tWorld");
	}
	
}

