//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

asm(".code16gcc");

#include "bios_tools.h"

#define PUSHB(V) asm("push %0" : : "m" (V) )
#define POPB(V) asm("pop %0" : "=m" (V) : )

#define BIOS_PRINT_CHAR(C) \
	asm ("movw $0x0001,%%bx\n" \
		 "movb %0,%%al\n" \
		 "movb $0x0e,%%ah\n" \
	     "int $0x10\n" \
		 : : "r" (C): "al")

void BIOS_print_char(byte_t ch) {
	BIOS_PRINT_CHAR(ch);
}

void BIOS_print_string(byte_t *str) {
	if (str) {
		char *s = str;
		while (*s) {
			BIOS_PRINT_CHAR(*s++);
		};
	}
}

void BIOS_print_number(long i, byte_t base) {	
	int v = i;
	char ch = '-';
	char n;
	char sz = 1;
	if (i < 0) {
		BIOS_PRINT_CHAR(ch);
		v = -v;
	}
	do {
		n = v % base;
		v /= base;
		if (n < 10)
			ch = n + 0x30;
		else
			ch = n + 0x37;
		PUSHB(ch);
		++sz;
	} while (v);
	do {
		POPB(ch);
		BIOS_PRINT_CHAR(ch);
		--sz;
	} while (sz);
}

#define BIOS_QUERY_CURSOR_POSITION(r,c) \
	asm("movb $0x03,%%ah\n" \
		"movb $0x00,%%bh\n" \
		"int $0x10\n" \
		"movb %%dh,%0\n" \
		"movb %%dl,%1\n" \
	: "=m"(r),"=m"(c) : : )

void BIOS_query_cursor_position(byte_t *row, byte_t *col) {
	BIOS_QUERY_CURSOR_POSITION(row,col);
}

#define BIOS_CHECK_KEY(x) \
	asm("movb $0x01,%%ah\n" \
		"int $0x16\n" \
		"jz 1f\n" \
		"movb $0x00, %0\n" \
		"1: movb %%ah, %0\n" \
	: "=r" (x) : : )

#define BIOS_GET_KEY(x) \
	asm("movb $0x00,%%ah\n" \
		"int $0x16\n" \
		"mov %%ax, %0\n" \
	: "=r" (x) : : )
	
byte_t BIOS_run_input_loop(
	BIOS_input_cb_t input_cb, 
	BIOS_no_input_cb_t no_input_cb
	)
{
	byte_t check = 0,
		 res = 0;
	word_t data;
	do {
		BIOS_CHECK_KEY(check);
		if (check && input_cb) {
			BIOS_GET_KEY(data);
			res = (*input_cb)(data>>8,data&0xff);
		}
		else if (no_input_cb) {
			res = (*no_input_cb)();
		}
		asm("nop");
	} while (!res);
	return res;
}

#define BIOS_RESET(mode) \
	asm ("mov $0x40,%%ax\n" \
		 "mov %%ax,%%ds\n" \
		 "movw %0,%%ds:(0x72)\n" \
		 "mov $0xf000,%%ax\n" \
		 "mov %%ax,%%cs\n" \
		 "jmpl %%cs:(0xfff0)" \
		: "=r" (mode) : : )

void BIOS_reset(word_t mode)
{
	BIOS_RESET(mode);
}

