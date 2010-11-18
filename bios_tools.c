//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

asm(".code16gcc");

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

