//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//
#ifndef BIOS_TOOLS_HEADER
#define BIOS_TOOLS_HEADER

#include "loader.h"
#include "loader_types.h"

/* Loader descriptor */
typedef struct loader_descriptor_s {
	word_t magic;
	byte_t version[3];
	byte_t loader_sectors_count;
} loader_descriptor_t;

/* Console out routes */
void BIOS_init_console_out(void *out); 

typedef byte_t (*BIOS_no_input_cb_t)(void);
typedef byte_t (*BIOS_input_cb_t)(byte_t/*scancode*/,byte_t/*ascii*/);

extern byte_t BIOS_run_input_loop(
	BIOS_input_cb_t input_cb, 
	BIOS_no_input_cb_t no_input_cb
	);

/* mode == 0x1234 || mode == 0 */
void BIOS_reset(word_t mode);

#endif//BIOS_TOOLS_HEADER

