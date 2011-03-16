#ifndef GDT_TABLE_HEADER
#define GDT_TABLE_HEADER

#include "loader_types.h"
#include "gdt_table.gen.h"

extern void calculate_gdt_address(void);
extern word_t	__gdtr;
extern dword_t	__gdt_addr;

#endif
