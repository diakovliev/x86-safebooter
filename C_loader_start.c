//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include <loader.h>
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

static inline void tools_memory_dump(void *addr, word_t size)
{
	word_t i;
	for (i=0; i < size; ++i) {
		if (i != 0) 
			puts(" ");		

		puts("0x");
		if (((byte_t*)addr)[i] < 0x10)
			puts("0");

		puts(itoa(((byte_t*)addr)[i],16));	
	}
}

static void tools_jump_to_kernel() 
{
	jump_to_kernel_asm();
}

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
		(void*)KERNEL_SETUP_ADDRESS, KERNEL_SETUP_SECTORS, KERNEL_CODE_LBA);
	if ( res != KERNEL_SETUP_SECTORS ) {
		/* IO error */
		puts(" FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	puts(" DONE\r\n");

	/* 2. Check kernel signature */
	kernel_header_p kernel_header = (kernel_header_p)GET_KERNEL_HEADER_ADDRESS(KERNEL_SETUP_ADDRESS);
	if ( kernel_header->header != KERNEL_HDRS ) {
		puts("Wrong kernel signature\r\n");
		return ERR_CMD_FAIL;
	}
	
	/* 3. Header analyze */
	if (!kernel_header->setup_sects) {
		kernel_header->setup_sects = 4;
	}
	
	puts("Supported LBP: 0x");
	puts(itoa(kernel_header->version,16));
	puts("\r\n");

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
		puts(" ");
		puts(itoa(sectors_count,10));
		puts(" ");
		puts(itoa(res,10));
		puts(" FAIL\r\n");
		return ERR_CMD_FAIL;
	}
	puts(" DONE\r\n");

	return ERR_CMD_OK;
}

byte_t IMAGE_boot(byte_t *cmd_buffer) {

	cmd_buffer = cmd_buffer;

	puts("Boot...\r\n");
	
	tools_jump_to_kernel();

	return ERR_CMD_OK;
}

/* Display memory */
byte_t TOOLS_help(byte_t *cmd_buffer) {

	cmd_command_t *command = commands;
	puts("command(alias) - help\r\n");
	puts("---------------------\r\n");
	while (command->name) {
		puts(command->name);
		puts("(");
		puts(command->alias);
		puts(") - ");
		puts(command->help);
		puts("\r\n");
		++command;
	}

	return ERR_CMD_OK;
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
	
	puts("address: 0x");
	puts(itoa(addr,16));
	puts("\r\nsize: ");
	puts(itoa(sz,10));
	puts("\r\n");
	tools_memory_dump((void*)addr, sz);
	puts("\r\n");
	
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
				puts("command error: ");
				puts(itoa(res,16));
				puts("\r\n");
				puts("Type 'help' for list available commands\r\n");
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

	puts("ATA(");
	puts(itoa(bus,16));
	puts(":");
	puts(itoa(drive,16));
	puts(") - ");

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

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{


	display_t d;
	keyboard_driver_t k;
	display_init(&d, (void*)TXT_VIDEO_MEM, 80, 25);
	keyboard_init(&k);
	display_clear(&d);
	console_init(display_get_console(&d,&k));

	/*
	word_t serial_port = COM1;
	ser_init(serial_port);
	console_init(ser_get_console(serial_port));
	*/

	/* Get loader descriptor information */
	loader_descriptor_p desc = (loader_descriptor_p)loader_descriptor_address;
	loader_descriptor = desc;
	
	/* Out information and command promt */
	puts("32bit SS loader v");
	puts(itoa(desc->version[0],10));
	putc('.');
	puts(itoa(desc->version[1],10));
	putc('.');
	puts(itoa(desc->version[2],10));
	puts(" (c)daemondzk@gmail.com");
	
	/* Memory map */
	puts("\r\nDescriptor: 0x");
	puts(itoa(loader_descriptor_address,16));
	puts("\r\nCode: 0x");
	puts(itoa(loader_code_address,16));
	puts("\r\nStack: 0x");
	puts(itoa(LOADER_STACK_ADDRESS,16));
	puts("\r\nLoader sectors: ");
	puts(itoa(desc->loader_sectors_count,10));
	puts("\r\nKernel sectors: ");
	puts(itoa(desc->kernel_sectors_count,10));	
	puts("\r\n");

	/* Detect ATA drives */
	puts("Detect ATA devices: \r\n");
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_PRIMARY, ATA_DRIVE_SLAVE);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_MASTER);
	detect_ata_drive(ATA_BUS_SECONDARY, ATA_DRIVE_SLAVE);

	puts(CMD_PROMT_INVITE);
	while (1) {
		C_input(getc());
	}
}

