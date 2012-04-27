#ifndef HEAP_HEADER
#define HEAP_HEADER

#include <stdint.h>
#include <loader_types.h>

/* Heap descriptor */
typedef struct heap_s {
	void *start;
	size_t size;
} heap_t, *heap_p;

/* API to create and work separated heap */
void *heap__malloc(heap_p heap, size_t size);
void heap__free(heap_p heap, void *ptr);
void heap__init(heap_p heap, void *start, size_t size);

/* Global heap API */
void heap_init(void *heap_start, size_t heap_size);
void *malloc(size_t size);
void free(void *ptr);

/* Debug */
void dump_heap_info(void);

#endif//HEAP_HEADER
