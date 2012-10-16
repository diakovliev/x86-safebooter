#ifndef HEAP_HEADER
#define HEAP_HEADER

#include <stdint.h>
#include <loader_types.h>

#ifdef DEBUG_HEAP
#define MAGIC_NODE_START	0xABCDEFAB
#define MAGIC_NODE_END		0xFEDCABBA
#endif/*DEBUG_HEAP*/

/* Heap item */
typedef struct heap_node_s {
#ifdef DEBUG_HEAP
	dword_t magic_node_start;
#endif/*DEBUG_HEAP*/
	void *start;
	void *raw_start; 
	size_t size;
	uint8_t busy;
	struct heap_node_s *prev,*next;
#ifdef DEBUG_HEAP
	dword_t magic_node_end;
#endif/*DEBUG_HEAP*/
} heap_node_t, *heap_node_p;

/* Heap descriptor */
typedef struct heap_s {
	void *start;
	size_t size;
	struct heap_node_s *first,*last;
#ifdef DEBUG_HEAP
	size_t busy_nodes_count;
	size_t alloc_count;
	size_t allocated;
	size_t free_nodes_count;
	size_t free_count;
	size_t released;
#endif
} heap_t, *heap_p;

/* API to create and work separated heap */
void *heap__malloc(heap_p heap, size_t size);
void *heap__memalign(heap_p heap, size_t align, size_t size);
void heap__free(heap_p heap, void *ptr);
void heap__init(heap_p heap, void *start, size_t size);

size_t heap__malloc_usable_size(heap_p heap, void *ptr);

/* Global heap API */
void heap_init(void *heap_start, size_t heap_size);
void *malloc(size_t size);
void *memalign(size_t align, size_t size);
void free(void *ptr);

size_t malloc_usable_size(void *ptr);

#if 0
/* Slub */
typedef struct slub_node_s {
	uint8_t busy;
	uint8_t data[];
} slub_node_t, *slub_node_p;

typedef struct slub_s {
	size_t isize;
	size_t icount;
	struct slub_node_s *first;
} slub_t, *slub_p;

int slub__init(slub_p slub, size_t isize, size_t icount)
{	
	slub->isize = isize;
	slub->icount = icount;
	slub->first = (slub_node_p)malloc((isize*icount) + icount);
	if (slub->first) {
		memset(slub->first,0,isize*icount + icount);
		return 0;
	}
	else
		return -1;
}

void *slub__alloc(slub_p slub)
{
	void *result = 0;
	size_t step = slub->isize + 1;
	void *current = slub->first;
	do {
		if (!((slub_node_p)current)->busy) {
			((slub_node_p)current)->busy = 1;
			result = &((slub_node_p)current)->data;
			break;
		}
		current += step;
	} while (slub->first + slub->icount > current);
	return result;
}

void slub__free(void *ptr)
{

}

#endif

/* Debug */
void dump_heap_info(void);

#endif//HEAP_HEADER
