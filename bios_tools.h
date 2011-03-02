//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//
#ifndef BIOS_TOOLS_HEADER
#define BIOS_TOOLS_HEADER

#include "loader.h"
#include "loader_types.h"

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

typedef struct disk_address_packet_s {
	byte_t	struct_size; /* should be 0x10 */
	byte_t	reserved;
	word_t	blocks;
	dword_t	buffer;
	quad_t	lba;
} disk_address_packet_t;
typedef disk_address_packet_t *disk_address_packet_p;

byte_t BIOS_read_storage_data(void *disk_address);

#endif//BIOS_TOOLS_HEADER

