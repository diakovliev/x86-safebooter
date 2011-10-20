#ifndef HEAP_HEADER
#define HEAP_HEADER

#include <stdint.h>
#include <loader_types.h>

struct heap_s {
	void *ctx;
	void *(*malloc)(void *ctx, size_t size);
	void (*free)(void *ctx, void *ptr);
	void (*dump_heap_info)(void *ctx);
} heap, heap_p;

void heap_init(void *heap_start, size_t heap_size);

void *malloc(size_t size);
void free(void *ptr);
void dump_heap_info(void);

#endif//HEAP_HEADER
