/*
 *	Project: <project>
 *
 *	File: <filename>
 *	Author: <author>
 *	Created: <created date>
 *
 *	Description:
 *
 *
 */

#include "loader.h"

asm(".code16gcc");

// ASM ROUTES
#define CLI __asm__ __volatile__ ("cli")
#define STI __asm__ __volatile__ ("sti")
#define NOP __asm__ __volatile__ ("nop")

#define PUSHB(V) asm("push %0" : : "m" (V) )
#define POPB(V) asm("pop %0" : "=m" (V) : )

#define BIOS_PRINT_CHAR(C) \
	asm ("movw $0x0001,%%bx\n" \
		 "movb %0,%%al\n" \
		 "movb $0x0e,%%ah\n" \
	     "int $0x10\n" \
		 : : "r" (C): "al")

void BIOS_print_char(char ch) {
	BIOS_PRINT_CHAR(ch);
}

void BIOS_print_string(char *str) {
	if (str) {
		char *s = str;
		do {
			BIOS_PRINT_CHAR(*s);
		} while (*s++);
	}
}

void BIOS_print_number(long i, unsigned char base) {	
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

void C_start(void) __attribute__((noreturn));
void C_stop(void) __attribute__((noreturn)); 

void C_stop(void) { 
	 asm("cli\n" 
		 "hlt\n");
}

void C_start(void) {

	BIOS_print_char('C');
	BIOS_print_char(':');
	BIOS_print_number(0xAA11,10);
	BIOS_print_char(':');

	/* Out hello */
	BIOS_print_string("Hello loader");

	C_stop();
}

