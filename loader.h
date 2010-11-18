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

#include "loader.gen.h"

/* Loader parameters block size */
#define BPB_SIZE			0x3e

/* Address where has been placed stack */
#define STACK_ADDRESS		0x2000

/* Message output */
#define MSG(x)				movw $(x),%si; call print_string

/* Message output routes */
#define PRINT_IMPL	\
print_char:;	\
	movb $0x0E,%ah;	\
	movb $0x00,%bh;	\
	movb $0x07,%bl;	\
	int $0x10;	\
	ret;	\
print_string:;	\
	mov (%si),%al; \
	inc %si;	\
	or %al,%al; \
	jz print_screen_exit_function; \
	call print_char; \
	jmp print_string; \
print_screen_exit_function:; \
	ret

