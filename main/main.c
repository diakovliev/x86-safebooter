//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
#include <heap.h>
#include <common.h>
#include <string.h>
#include <lbp.h>
#include <jump_to_kernel.h>

#include <drivers/console_iface.h>
#include <drivers/text_display_driver.h>
#include <drivers/text_display_console.h>
#include <drivers/keyboard_driver.h>
#include <drivers/ascii_driver.h>
#include <drivers/ata_driver.h>
#include <drivers/serial_driver.h>

/* TODO: Move to command processor header */
#define ERR_CMD_OK				0
#define ERR_CMD_FAIL			ERR_CMD_OK+1
#define ERR_CMD_NOT_SUPPORTED	ERR_CMD_OK+2
#define ERR_CMD_BAD_PARAMS		ERR_CMD_OK+3

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

/* forward declarations */
byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer);
byte_t IMAGE_boot(byte_t *cmd_buffer);
byte_t TOOLS_display_memory(byte_t *cmd_buffer);
byte_t TOOLS_help(byte_t *cmd_buffer);

/* Registered commands */
static cmd_command_t commands[] = {
	{"help", "h", "list available commands", TOOLS_help},
	{"load", "l", "load kernel to memory", IMAGE_load_kernel_to_memory},
	{"boot", "b", "boot kernel", IMAGE_boot},
	{"display", "d", "display memory", TOOLS_display_memory},

	/* last element */
	{0,0,0,0},
};

byte_t IMAGE_load_kernel_to_memory(byte_t *cmd_buffer)
{
	cmd_buffer = cmd_buffer;
	word_t res = 0;

	word_t ata_bus		= ATA_BUS_PRIMARY;
	byte_t ata_drive	= ATA_DRIVE_MASTER;

	/* Load real mode kernel */

	/* 1. Load first KERNEL_SETUP_SECTORS sectors */
	puts("Loading kernel header ");
	res = ata_read_sectors(ata_bus, ata_drive, 
		(void*)KERNEL_REAL_CODE_ADDRESS, KERNEL_SETUP_SECTORS, KERNEL_CODE_LBA);
	if ( res != KERNEL_SETUP_SECTORS ) {
		/* IO error */
		puts(" FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	puts(" DONE\r\n");

	/* 2. Check kernel signature */
	kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(KERNEL_REAL_CODE_ADDRESS);
	if ( kernel_header->header != KERNEL_HDRS ) {
		puts("Wrong kernel signature\r\n");
		return ERR_CMD_FAIL;
	}
	
	/* 3. Header analyze */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}
	
	printf("Supported LBP: 0x%x\r\n", kernel_header->version);

	/* 3. Load realmode kernel */
	puts("Loading real mode kernel ");
	res = ata_read_sectors(ata_bus, ata_drive, 
		(void*)KERNEL_REAL_CODE_ADDRESS, kernel_header->setup_sects+1, KERNEL_CODE_LBA);

	if ( res != kernel_header->setup_sects+1 ) {
		/* IO error */
		puts(" FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	puts(" DONE\r\n");

	/* correct setup_sects */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}

	word_t sectors_count = loader_descriptor->kernel_sectors_count - (kernel_header->setup_sects + 1); 
	/* 4. Load protected mode kernel */
	puts("Loading protected mode kernel");
	res = ata_read_sectors(ata_bus, ata_drive
		, (void*)KERNEL_CODE_ADDRESS
		, sectors_count
		, KERNEL_CODE_LBA + kernel_header->setup_sects + 1);

	if ( res != sectors_count ) {
		/* IO error */
		printf("FAIL (sectors_count: %d, res: %d)\r\n", sectors_count, res);
		return ERR_CMD_FAIL;
	}
	puts(" DONE\r\n");

	return ERR_CMD_OK;
}

byte_t IMAGE_boot(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	puts("Boot...\r\n");
	
	jump_to_kernel_asm();

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

/* Command processor entry point */
byte_t C_process_command(byte_t *cmd_buffer) {
	byte_t r = ERR_CMD_NOT_SUPPORTED;

	strtok(CMD_CMD_SEP, cmd_buffer );
	byte_t *buf = 0;	
	byte_t buffer[CMD_BUFFER_MAX];

	while ( buf = strtok(CMD_CMD_SEP,0) ) {
		buf = strltrim(" ",buf);
		strcpy(buffer,buf);
		
		cmd_command_t *command = commands;
		while (command->name) {
			if ( 	starts_from(buffer, command->name) || 
					starts_from(buffer, command->alias) ) {
				r = (*command->function)(buffer);
			}
			++command;
		}		
	}

	return r;
}

/* Input loop callback */
byte_t C_input(byte_t ascii) {
	
	static byte_t cmd_buffer[CMD_BUFFER_MAX];
	static byte_t *pos = cmd_buffer;

	if (ascii == '\r') {
		puts("\r\n");
		if ( pos != cmd_buffer ) {
			byte_t res = C_process_command(cmd_buffer);
			if (res) {
				// TODO: Out symbolic error code 
				printf("Command error: %x\n\rType 'help' for list available commands\r\n", res);
			}				
		}
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

/* Detect ATA */
void detect_ata_drive(word_t bus, byte_t drive) {

	printf("ATA(%x:%x) - ", bus, drive);

	switch (ata_identify_device(bus,drive)) {
	case ATADEV_NONE:
		puts("none");
		break;
	case ATADEV_PATA:
		puts("PATA");
		break;
	case ATADEV_SATA:
		puts("SATA");
		break;
	case ATADEV_PATAPI:
		puts("PATAPI");
		break;
	case ATADEV_SATAPI:
		puts("SATAPI");
		break;
	default:
		puts("Unknown");
	}

	puts("\r\n");
}

extern void test_gmp(void);

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	heap_init();
	
#ifdef CONFIG_CONSOLE_ENABLED
#	ifdef CONFIG_CONSOLE_SERIAL	
	word_t serial_port = CONFIG_CONSOLE_SERIAL_PORT;
	ser_init(serial_port);
	console_init(ser_get_console(serial_port));
#	else//CONFIG_CONSOLE_SERIAL
	display_t d;
	keyboard_driver_t k;
	display_init(&d, (void*)TXT_VIDEO_MEM, 80, 25);
	keyboard_init(&k);
	display_clear(&d);
	console_init(display_get_console(&d,&k));
#	endif//CONFIG_CONSOLE_SERIAL
#endif//CONFIG_CONSOLE_ENABLED

	/* Get loader descriptor information */
	loader_descriptor_p desc = (loader_descriptor_p)loader_descriptor_address;
	loader_descriptor = desc;
	
	/* Out information and command promt */
	printf("32bit SS loader v%d.%d.%d (c)daemondzk@gmail.com\r\n", 
		desc->version[0], 
		desc->version[1],
		desc->version[2]);
	
	/* Memory map */
	printf("Descriptor: %p\r\n", loader_descriptor_address);
	printf("Code: %p\r\n", loader_code_address);
	printf("Stack: %p\r\n", LOADER_STACK_ADDRESS);
	printf("Loader sectors: %d\r\n", desc->loader_sectors_count);
	printf("Kernel sectors: %d\r\n", desc->kernel_sectors_count);	

	/* Detect ATA drives */
	printf("Detect ATA devices:\r\n");
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE);

	test_gmp();

	puts(CMD_PROMT_INVITE);
	while (1) {
		C_input(getc());
	}
}
