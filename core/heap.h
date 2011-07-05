#ifndef HEAP_HEADER
#define HEAP_HEADER

#include <stdint.h>

#define size_t uint32_t

extern void heap_init(void);
extern void *malloc(size_t size);
extern void free(void *ptr);
extern void dump_heap_info(void);

#endif//HEAP_HEADER
