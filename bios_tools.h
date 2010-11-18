//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

/* Common types */
typedef unsigned char byte_t;
typedef unsigned short word_t;

extern void BIOS_print_char(byte_t ch);
extern void BIOS_print_string(byte_t *str);
extern void BIOS_print_number(long i, byte_t base);

typedef byte_t (*BIOS_no_input_cb_t)(void);
typedef byte_t (*BIOS_input_cb_t)(byte_t/*scancode*/,byte_t/*ascii*/);

extern byte_t BIOS_run_input_loop(
	BIOS_input_cb_t input_cb, 
	BIOS_no_input_cb_t no_input_cb
	);
