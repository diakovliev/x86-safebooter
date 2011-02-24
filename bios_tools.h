//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//
#include "loader.h"

/* Common types */
typedef unsigned char byte_t;
typedef unsigned short word_t;
typedef unsigned int dword_t;

/* Loader descriptor */
typedef struct loader_descriptor_s {
	word_t magic;
	byte_t version[3];
	byte_t loader_sectors_count;
} loader_descriptor_t;

/**/
extern void BIOS_print_char(byte_t ch);
extern void BIOS_print_string(byte_t *str);
extern void BIOS_print_number(long i, byte_t base);

typedef byte_t (*BIOS_no_input_cb_t)(void);
typedef byte_t (*BIOS_input_cb_t)(byte_t/*scancode*/,byte_t/*ascii*/);

extern byte_t BIOS_run_input_loop(
	BIOS_input_cb_t input_cb, 
	BIOS_no_input_cb_t no_input_cb
	);

/* mode == 0x1234 || mode == 0 */
void BIOS_reset(word_t mode);
