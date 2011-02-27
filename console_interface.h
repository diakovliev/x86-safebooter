//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2011
//
#ifndef CONSOLE_INTERFACE_HEADER
#define CONSOLE_INTERFACE_HEADER

#include "loader.h"
#include "loader_types.h"

typedef struct console_out_s {
	void (*out_char) (byte_t);
	void (*out_string) (byte_p);
	void (*out_number) (long,byte_t);
} console_out_t;
typedef struct console_out_s *console_out_p;

/* Console out interface */
extern console_out_p out;
#define O(METHOD,...) if ( out->out_##METHOD ) (*out->out_##METHOD) (__VA_ARGS__)

#endif//CONSOLE_INTEFRACE_HEADER

