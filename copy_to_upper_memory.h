#ifndef COPY_TO_UPPER_MEMORY_HEADER
#define COPY_TO_UPPER_MEMORY_HEADER

#include "loader_types.h"

extern void copy_to_upper_memory_asm(dword_t dst,dword_t src,dword_t size);

extern dword_t p_mode_dst;
extern dword_t p_mode_src;
extern dword_t p_mode_size;

#endif
