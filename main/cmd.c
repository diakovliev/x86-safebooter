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

#include <string.h>
#include "cmd.h"

static byte_p errors[] = {
	[ERR_CMD_OK]			= "OK",
	[ERR_CMD_FAIL]			= "FAIL",
	[ERR_CMD_NOT_SUPPORTED]	= "UNKNOWN COMMAND",
	[ERR_CMD_BAD_PARAMS]	= "BAD COMMAND PARAMETERS",
};

/* Registered commands */
static cmd_command_t *commands = 0;

/* Registed command array */
cmd_command_t *cmd_register_commands(cmd_command_t *cmds) {
	if (commands != cmds) commands = cmds;
	return commands;
}

const byte_t *cmd_error(byte_t err) {
	return errors[err];
}

/* Command processor entry point */
byte_t cmd_process_command(byte_t *cmd) {

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
		
		/*printf("\n\r======================\n\r");*/

		cmd_command_t *command = commands;
		while (command->name) {
		
			/*printf("\n\r '%s' '%s' '%s' \n\r", command->name, command->alias, buffer);*/

			if (starts_from(buffer, command->name) ||
				starts_from(buffer, command->alias) ) {

				/*printf("\n\r==> '%s'\n\r", command->name);*/

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
byte_t cmd_input(byte_t ascii) {

	static byte_t init = 1;	
	static byte_t cmd_buffer[CMD_BUFFER_MAX];
	static byte_t *pos = cmd_buffer;

	if (init) {
		/*print_current_time();*/
		puts(CMD_PROMT_INVITE);
		init = 0;
	}

	if (ascii == '\r') {
		puts("\r\n");
		if ( pos != cmd_buffer ) {
			byte_t res = cmd_process_command(cmd_buffer);
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

#endif/*CONFIG_COMMAND_LINE_ENABLED*/

