//
// This file is part of project
// "32bit secured bootloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
#include <heap.h>
#include <env.h>
#include <common.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <image.h>

#include <crypt/bch.h>

#include <drivers/text_display_driver.h>
#include <drivers/text_display_console.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <drivers/serial_driver.h>

/* forward declarations */
byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer);
byte_t IMAGE_boot(byte_t *cmd_buffer);
byte_t TOOLS_display_memory(byte_t *cmd_buffer);
byte_t TOOLS_help(byte_t *cmd_buffer);
byte_t TOOLS_heap_info(byte_t *cmd_buffer);
byte_t TOOLS_print_env(byte_t *cmd_buffer);

/* Max command identifier size */
#define CMD_BUFFER_MAX 0x20

/* Command promt */
#define CMD_PROMT_INVITE	">> "
#define CMD_PARAM_SEP		" "
#define CMD_CMD_SEP			";"

/* Pointer to loader descriptor */
static loader_descriptor_p loader_descriptor = 0;

/* Command line command */
typedef struct cmd_command_s {
	byte_t *name;
	byte_t *alias;
	byte_t *help;
	byte_t (*function)(byte_t *);
} cmd_command_t;

/* Errors */
static byte_p errors[] = {
#define ERR_CMD_OK				0
	[ERR_CMD_OK]			= "OK",
#define ERR_CMD_FAIL			ERR_CMD_OK+1
	[ERR_CMD_FAIL]			= "FAIL",
#define ERR_CMD_NOT_SUPPORTED	ERR_CMD_OK+2
	[ERR_CMD_NOT_SUPPORTED]	= "UNKNOWN COMMAND",
#define ERR_CMD_BAD_PARAMS		ERR_CMD_OK+3
	[ERR_CMD_BAD_PARAMS]	= "BAD COMMAND PARAMETERS",
};

/* Registered commands */
static cmd_command_t commands[] = {
	{"help", "h", "list available commands", TOOLS_help},
	{"load", "l", "load kernel to memory", IMAGE_load_kernel_to_memory},
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
	blk_istream_p s = ata_blk_stream_open(bus,drive,lba);
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

/* Command processor entry point */
byte_t C_process_command(byte_t *cmd) {

	byte_t r = ERR_CMD_NOT_SUPPORTED;
	byte_t cmd_buffer[CMD_BUFFER_MAX];
	byte_t buffer[CMD_BUFFER_MAX];
	byte_t *buf = 0;
	byte_t *safe_buf = 0;

	strcpy(cmd_buffer,cmd);
	strtok(CMD_CMD_SEP,cmd_buffer);

	while ( buf = strtok(CMD_CMD_SEP,0) ) {
		buf = strltrim(" ",buf);
		strcpy(buffer,buf);
		
		cmd_command_t *command = commands;
		while (command->name) {
			if (starts_from(buffer, command->name) ||
				starts_from(buffer, command->alias) ) {

				safe_buf = strtok(CMD_CMD_SEP,buf);
				r = (*command->function)(buffer);
				strtok(CMD_CMD_SEP,safe_buf);

				break;
			}
			++command;
		}

	}

	return r;
}

#ifdef CONFIG_COMMAND_LINE_ENABLED

/* Input loop callback */
byte_t C_input(byte_t ascii) {
	
	static byte_t cmd_buffer[CMD_BUFFER_MAX];
	static byte_t *pos = cmd_buffer;

	if (ascii == '\r') {
		puts("\r\n");
		if ( pos != cmd_buffer ) {
			byte_t res = C_process_command(cmd_buffer);
			if (res) {
				printf("Command error: %s\r\n", errors[res]);
			}				
		}

		/*print_current_time();*/
		puts(CMD_PROMT_INVITE);

		/* Reset buffer */
		pos		= cmd_buffer;
		*pos	= 0;
	}
	else {
		/* command buffer overflow protection */
		if ( (cmd_buffer + CMD_BUFFER_MAX) >= (pos + 1) ) {
			*pos	= ascii;
			*(pos+1)= 0;
			++pos;
		}
		putc(ascii);
	}

	return 0;
}

#endif//CONFIG_COMMAND_LINE_ENABLED

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
	heap_init((void*)LOADER_HEAP_START,LOADER_HEAP_SIZE);
	env_init(desc);
	console_initialize();

	/* Out information and command promt */
	printf("32bit secured bootloader v%d.%d.%d (c)daemondzk@gmail.com\r\n", 
		desc->version[0], 
		desc->version[1],
		desc->version[2]);
	
	/* Memory map */
	printf("Descriptor: %p\r\n", loader_descriptor_address);
	printf("Code: %p\r\n", loader_code_address);
	printf("Stack: %p\r\n", LOADER_STACK_ADDRESS);

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

	/* Run environment STARTUP commands */
	byte_t *startup = env_get("STARTUP");
	if (!ctrl_break && startup) {
		printf("Process STARTUP commands...\n\r");
		byte_t res = C_process_command(startup);
		if (res) {
			printf("STARTUP commands error: %s\n\r", errors[res]);
		}
	} else {
		printf("BREAK revieved, ignore STARTUP variable\n\r");
	}

#ifdef CONFIG_CONSOLE_ENABLED
	/*print_current_time();*/
#ifdef CONFIG_COMMAND_LINE_ENABLED
	puts(CMD_PROMT_INVITE);
	while (1) {
		C_input(getc());
	}
#endif//CONFIG_COMMAND_LINE_ENABLED

#endif//CONFIG_CONSOLE_ENABLED

}

