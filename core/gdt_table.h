#ifndef GDT_TABLE_HEADER
#define GDT_TABLE_HEADER

#include "loader_types.h"
#include "gdt_table.gen.h"

extern void init_gdt(void);
extern word_t	gdtr;
extern dword_t	gdt_addr;

#endif
