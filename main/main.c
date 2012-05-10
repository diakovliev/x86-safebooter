//
// This file is part of project
// "32bit secured bootloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
#include <heap.h>
#include <env.h>
#include <common.h>
#include <debug.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <image.h>

#include <drivers/text_display_driver.h>
#include <drivers/text_display_console.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <drivers/serial_driver.h>

#include "cmd.h"

/* forward declarations */
byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer);
byte_t IMAGE_load_to_memory(byte_t *cmd_buffer);
byte_t IMAGE_boot(byte_t *cmd_buffer);
byte_t TOOLS_display_memory(byte_t *cmd_buffer);
byte_t TOOLS_help(byte_t *cmd_buffer);
byte_t TOOLS_heap_info(byte_t *cmd_buffer);
byte_t TOOLS_print_env(byte_t *cmd_buffer);

/* Pointer to loader descriptor */
static loader_descriptor_p loader_descriptor = 0;

/* Registered commands */
static cmd_command_t commands[] = {
	{"help", "h", "list available commands", TOOLS_help},
	{"kernelload", "k", "load kernel to memory", IMAGE_load_kernel_to_memory},
	{"load", "l", "load data to memory", IMAGE_load_to_memory},
	{"boot", "b", "boot kernel", IMAGE_boot},
	{"display", "d", "display memory", TOOLS_display_memory},
	{"heapinfo", "e", "show heap info", TOOLS_heap_info},
	{"printenv", "p", "show environment", TOOLS_print_env},

	/* last element */
	{0,0,0,0},
};

byte_t *strdup(byte_t *src) {
	word_t sz = strlen(src) + 1;
	byte_t *ptr = malloc(sz);
	memcpy(ptr,src,sz);
	return ptr;
}

byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer)
{
	byte_t res = ERR_CMD_FAIL;

	byte_t *buf = strtok(CMD_PARAM_SEP, cmd_buffer);
	strtok(CMD_PARAM_SEP, 0);

	byte_t *index_s = strtok(CMD_PARAM_SEP, 0);
	if (!index_s) {
		printf("Unknown image index\n\r");
		return res;
	}

	word_t index = atol(index_s,16);
	if (index < 0 || index > 3) {
		printf("Image index should be in [0..3]\n\r");
		return res;
	}

	/* Parse image info */
	byte_t env_name[32];
	memset(env_name, 0, sizeof(env_name));
	sprintf(env_name, "IMAGE_%d", index);
	byte_t *env_s = strdup(env_get(env_name));
	printf("Image: \"%s\"\n\r", env_s);

	strtok(":", env_s);
	byte_p type_s = strtok(":", 0);
	if (!type_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	byte_t image_type = *type_s;
	if (image_type != 'R' && image_type != 'S') {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	byte_p bus_s = strtok(":", 0);
	if (!bus_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	word_t bus = atol(bus_s,16);
	byte_p drive_s = strtok(":", 0);
	if (!drive_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	word_t drive = atol(drive_s,16);
	byte_p lba_s = strtok(":", 0);
	if (!lba_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	dword_t lba = atol(lba_s,16);
	byte_p size_s = strtok(":", 0);
	if (!size_s && *type_s == 'R') {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	dword_t size = atol(size_s,16);
	blk_iostream_p s = ata_blk_stream_open(bus,drive,lba);
	if (!s) {
		printf("Unable to open input stream\n\r");
		goto finish;
	}

	if (image_type == 'R') {
#ifdef CONFIG_RAW_IMAGES_ENABLED
		res = image_load_raw(s, lba, size) == 0 ? ERR_CMD_OK : ERR_CMD_FAIL;
#else
		printf("No support of RAW images\n\r");
#endif/*CONFIG_RAW_IMAGES_ENABLED*/
	} else
	if (image_type == 'S') {
		res = image_load_sig(s, lba) == 0 ? ERR_CMD_OK : ERR_CMD_FAIL;
	}

	ata_blk_stream_close(s);

finish:
	free(env_s);
	return res;
}

byte_t IMAGE_load_to_memory(byte_t *cmd_buffer)
{
	byte_t res = ERR_CMD_FAIL;

	byte_t *buf = strtok(CMD_PARAM_SEP, cmd_buffer);
	strtok(CMD_PARAM_SEP, 0);

	byte_t *index_s = strtok(CMD_PARAM_SEP, 0);
	if (!index_s) {
		printf("Unknown image index\n\r");
		return res;
	}

	word_t index = atol(index_s,16);
	if (index < 0 || index > 3) {
		printf("Image index should be in [0..3]\n\r");
		return res;
	}

	/* Parse image info */
	byte_t env_name[32];
	memset(env_name, 0, sizeof(env_name));
	sprintf(env_name, "IMAGE_%d", index);
	byte_t *env_s = strdup(env_get(env_name));
	printf("Image: \"%s\"\n\r", env_s);

	byte_t *address_s = strtok(CMD_PARAM_SEP, 0);
	if (!address_s) {
		printf("Unknown destination address\n\r");
		return res;
	}

	dword_t address = atol(address_s, 16);
	printf("Address: %p\n\r", address);

	strtok(":", env_s);
	byte_p type_s = strtok(":", 0);
	if (!type_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	byte_t image_type = *type_s;
	if (image_type != 'R' && image_type != 'S') {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	byte_p bus_s = strtok(":", 0);
	if (!bus_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	word_t bus = atol(bus_s,16);
	byte_p drive_s = strtok(":", 0);
	if (!drive_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	word_t drive = atol(drive_s,16);
	byte_p lba_s = strtok(":", 0);
	if (!lba_s) {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	dword_t lba = atol(lba_s,16);
	byte_p size_s = strtok(":", 0);
	if (!size_s && *type_s == 'R') {
		printf("Wrong environment variable \"%s\" format\n\r", env_name);
		goto finish;
	}
	dword_t size = atol(size_s,16);
	blk_iostream_p s = ata_blk_stream_open(bus,drive,lba);
	if (!s) {
		printf("Unable to open input stream\n\r");
		goto finish;
	}

	if (image_type == 'R') {
#ifdef CONFIG_RAW_IMAGES_ENABLED
		/*res = image_load_raw(s, lba, size) == 0 ? ERR_CMD_OK : ERR_CMD_FAIL;*/
		printf("No support of RAW images\n\r");
#else
		printf("No support of RAW images\n\r");
#endif/*CONFIG_RAW_IMAGES_ENABLED*/
	} else
	if (image_type == 'S') {
		int load_res = 0;
		printf("Loading data... ");
		load_res = load_simg((void*)address, s) == 0 ? ERR_CMD_OK : ERR_CMD_FAIL;
		if (load_res > 0) {
			printf("[%p - %p] DONE\n\r", address, (address + size));
		} else {
			puts("FAIL\n\r");
		}
	}

	ata_blk_stream_close(s);

finish:
	free(env_s);
	return res;
}

byte_t IMAGE_boot(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	image_boot(loader_descriptor);

	return ERR_CMD_OK;
}

/* Display memory */
byte_t TOOLS_help(byte_t *cmd_buffer) {

	cmd_command_t *command = commands;
	puts("command (alias) - help\r\n---------------------\r\n");
	while (command->name) {
		printf("%s (%s) - %s\r\n", command->name, command->alias, command->help);
		++command;
	}

	return ERR_CMD_OK;
}

static inline void tools_memory_dump(void *addr, word_t size)
{
	word_t i, j;
	byte_t b;

#define DUMP_WIDTH 8
	word_t count = size / DUMP_WIDTH;
	word_t finish = size % DUMP_WIDTH;

	for (i = 0; i < count * DUMP_WIDTH; i+= DUMP_WIDTH) {
		puts("\t");
		for (j = 0; j < DUMP_WIDTH; ++j) {
			b = ((byte_t*)addr)[i+j];
			printf(b < 0x10 ? "0x0%x " : "0x%x ", b);
		}
		puts("\n\r");
	}
	if (finish) puts("\t");
	for ( ;i < size; ++i) {
		b = ((byte_t*)addr)[i];
		printf(b < 0x10 ? "0x0%x " : "0x%x ", b);
	}
	if (finish) puts("\n\r");
#undef DUMP_WIDTH
}

/* Display memory */
byte_t TOOLS_display_memory(byte_t *cmd_buffer) {

	/* store old strtok buffer */	
	byte_t *buf = strtok(CMD_PARAM_SEP, cmd_buffer);
	strtok(CMD_PARAM_SEP, 0);

	byte_t *addr_s = strtok(CMD_PARAM_SEP, 0);
	if (!addr_s)
		return ERR_CMD_BAD_PARAMS;

	dword_t addr = atol(addr_s, 16);
	
	byte_t *sz_s = strtok(CMD_PARAM_SEP, 0);
	if (!sz_s)
		return ERR_CMD_BAD_PARAMS;
	
	word_t sz = atol(sz_s, 10);	
	
	printf("Address: %p\r\nSize: %d\r\n", addr, sz);
	tools_memory_dump((void*)addr, sz);
	
	/* restore old strtok buffer */
	strtok(CMD_CMD_SEP,buf);

	return ERR_CMD_OK;	
}

/* Heap info */
byte_t TOOLS_heap_info(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	dump_heap_info();

	return ERR_CMD_OK;
}

/* Environment */
byte_t TOOLS_print_env(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	env_print();

	return ERR_CMD_OK;
}

/* Environment */
byte_t TOOLS_set_env(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	env_print();

	return ERR_CMD_OK;
}

/* Console init code */
void console_initialize(void) {
	/* Console init */
#ifdef CONFIG_CONSOLE_ENABLED
#	ifdef CONFIG_CONSOLE_SERIAL
	word_t serial_port = CONFIG_CONSOLE_SERIAL_PORT;
	ser_init(serial_port);
	console_init(ser_get_console(serial_port));
#	else//CONFIG_CONSOLE_SERIAL
	static display_t d;
	static keyboard_driver_t k;
	display_init(&d, (void*)TXT_VIDEO_MEM, 80, 25);
	keyboard_init(&k);
	display_clear(&d);
	console_init(display_get_console(&d,&k));
#	endif//CONFIG_CONSOLE_SERIAL
#endif//CONFIG_CONSOLE_ENABLED
}

/* 32 bit C code entry point */
void C_start(void *, void *) __attribute__((noreturn));
void C_start(void *loader_descriptor_address, void *loader_code_address)
{
	/* Get loader descriptor information */
	loader_descriptor_p desc = (loader_descriptor_p)loader_descriptor_address;
	loader_descriptor = desc;

	/* Init subsystems */
	rtc_init();
	time_init();
	console_initialize();

	/* Out startup info */
	BUG_if_not(desc->version[0] == VER_MAJ);
	BUG_if_not(desc->version[1] == VER_MID);
	BUG_if_not(desc->version[2] == VER_MIN);

	printf("32bit secured bootloader v%d.%d.%d (c)daemondzk@gmail.com\r\n", 
		desc->version[0], 
		desc->version[1],
		desc->version[2]);
	
	/* Memory map */
#ifdef __DEBUG__
	printf("Descriptor: %p\r\n", loader_descriptor_address);
	printf("Code: %p\r\n", loader_code_address);
	printf("Stack: %p\r\n", LOADER_STACK_ADDRESS);
#endif/*__DEBUG__*/

	/* Waiting for break */
	byte_t ctrl_break = 0;
	quad_t tm = 5;
	byte_t c = 0;
	byte_t recv = 0;
	printf("Waiting %ld seconds for the BREAK command\n\r", tm);
	while (recv = waitc(&tm, &c)) {
		if (c == 'B') {
			ctrl_break = 1;
			break;
		}
	}

	/* Init heap */
	heap_init((void*)LOADER_HEAP_START,LOADER_HEAP_SIZE);
	/* Init environment */
	env_init(desc);
	/* Register commands set */
	cmd_register_commands(commands);

	/* Run environment STARTUP commands */
	if (!ctrl_break) {
		byte_t *startup = env_get("STARTUP");
		if (!startup) {
			printf("STARTUP variable is not set\n\r");
		} else {
			byte_t res = cmd_process_command(startup);
			if (res) {
				printf("STARTUP commands error: %s\n\r", cmd_error(res));
			}
		}
	} else {
		printf("BREAK revieved, ignore STARTUP variable\n\r");
	}

#ifdef CONFIG_CONSOLE_ENABLED
#ifdef CONFIG_COMMAND_LINE_ENABLED
	while (1) {
		cmd_input(getc());
	}
#endif//CONFIG_COMMAND_LINE_ENABLED
#endif//CONFIG_CONSOLE_ENABLED
	/* noreturn */
	while (1) {
		idle();
	}
}

