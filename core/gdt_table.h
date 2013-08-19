#ifndef GDT_TABLE_HEADER
#define GDT_TABLE_HEADER

#include "loader_types.h"

extern void init_gdt(void);
extern word_t	gdtr;
extern dword_t	gdt_addr;
extern dword_t	gdt_code_segment_addr;
extern dword_t	gdt_data_segment_addr;
extern dword_t	gdt_r_code_segment_addr;
extern dword_t	gdt_r_data_segment_addr;

#endif
