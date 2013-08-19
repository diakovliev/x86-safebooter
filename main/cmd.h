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
#ifndef CMD_HEADER
#define CMD_HEADER

#include <config.h>
#include <loader.h>
#include <loader_types.h>

/* Max command identifier size */
#define CMD_BUFFER_MAX 0x20

/* Command promt */
#define CMD_PROMT_INVITE	">> "
#define CMD_PARAM_SEP		" "
#define CMD_CMD_SEP			";"

/* Errors */
#define ERR_CMD_OK				0
#define ERR_CMD_FAIL			ERR_CMD_OK+1
#define ERR_CMD_NOT_SUPPORTED	ERR_CMD_OK+2
#define ERR_CMD_BAD_PARAMS		ERR_CMD_OK+3

/* Command line command */
typedef struct cmd_command_s {
	byte_t *name;
	byte_t *alias;
	byte_t *help;
	byte_t (*function)(byte_t *);
} cmd_command_t;

/* API */
cmd_command_t *cmd_register_commands(cmd_command_t *cmds);
const byte_t *cmd_error(byte_t err);
byte_t cmd_process_command(byte_t *cmd);
#ifdef CONFIG_COMMAND_LINE_ENABLED
byte_t cmd_input(byte_t ascii);
#endif

#endif/*CMD_HEADER*/

