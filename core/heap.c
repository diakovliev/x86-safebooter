#include "heap.h"
#include <loader.h>

#include <stdio.h>

/* Simple heap */

#ifndef HEAP_MAX
#define HEAP_MAX(x,y) (x)>(y)?(x):(y)
#endif//HEAP_MAX
#ifndef HEAP_MIN
#define HEAP_MIN(x,y) (x)<(y)?(x):(y)
#endif//HEAP_MIN

/* Heap item */
typedef struct heap_node_s {
	void 	*start;
} heap_node_t, *heap_node_p;

/* Control block */
typedef struct heap_ctl_s {
	uint8_t busy;
	size_t size;
} heap_ctl_t, *heap_ctl_p;

/*
LOADER_HEAP_START 
-----------------------------------------------------------------
		

	
					WILL USED HEAP AREA





LOADER_HEAP_START + LOADER_HEAP_SIZE
-----------------------------------------------------------------
					heap_node_s[0]
					heap_node_s[1]
						...
					heap_node_s[n-1]
					heap_node_s[n]
*/

/* Current size of nodes array */
static size_t heap_nodes_count = 0;

/* Pointer to nodes array */
static heap_node_p heap_nodes = 0;

/* Statistic */
static size_t max_alloc_size = 0;
static size_t min_alloc_size = 0;
static size_t total_allocated = 0;
static size_t total_released = 0;
static uint32_t total_allocs = 0;
static uint32_t total_frees = 0;

#define NODE_CTL(p) ((heap_ctl_p)(p->start - sizeof(heap_ctl_t)))

/* Initialize heap */
void heap_init(void *heap_start, size_t heap_size)
{
	//heap_nodes			= (heap_node_p)(LOADER_HEAP_START + LOADER_HEAP_SIZE);
	heap_nodes			= (heap_node_p)(heap_start + heap_size);
	heap_nodes_count	= 0;
}

/* find free nodes */
static heap_node_p get_free_node(size_t size) 
{
	heap_node_p current_node = heap_nodes;
	void *address = (void*)LOADER_HEAP_START;
	if (!heap_nodes_count) {
		++heap_nodes_count;
		current_node->start = address + sizeof(heap_ctl_t);
		NODE_CTL(current_node)->size = size;
		return current_node;	
	}
	while (heap_nodes + heap_nodes_count > current_node) {
		heap_ctl_p node_ctl = NODE_CTL(current_node);
		if (!node_ctl->busy && node_ctl->size >= size)
			return current_node;
		else {
			address = current_node->start + node_ctl->size + sizeof(heap_ctl_t);
		}
		++current_node;
	}
	if ((heap_node_p)(address + size + sizeof(heap_ctl_t)) < heap_nodes) {
		current_node = heap_nodes + heap_nodes_count;
		++heap_nodes_count;
		current_node->start = address + sizeof(heap_ctl_t);
		NODE_CTL(current_node)->size = size;
		return current_node;	
	}
	return 0;
}

void *malloc(size_t size)
{
	heap_node_p node = get_free_node(size);
	if (node) {
		NODE_CTL(node)->busy = 1;

//		printf("+++ Allocated %d bytes at %p\n\r", size, node->start);

		/* collect stats */
		max_alloc_size = HEAP_MAX(max_alloc_size,NODE_CTL(node)->size);
		min_alloc_size = !min_alloc_size ? NODE_CTL(node)->size : HEAP_MIN(min_alloc_size,NODE_CTL(node)->size);
		total_allocated += NODE_CTL(node)->size;
		++total_allocs;

		return node->start;	
	}

	printf("!!! Allocation failed !!!\n\r");

	return 0;
}

void free(void *ptr)
{
	heap_node_p current_node = heap_nodes;
	while (current_node->start != ptr && current_node < heap_nodes + heap_nodes_count) {
		++current_node;
	}
	if (!current_node) {
		printf("!!! Was tried to free unknown node at %p !!!\n\r", ptr);
		return;
	}

//	printf("--- Relesed %d bytes at %p\n\r", NODE_CTL(current_node)->size, current_node->start);

	/* collect stats */
	total_released += NODE_CTL(current_node)->size;
	++total_frees;

	NODE_CTL(current_node)->busy = 0;
}

void dump_heap_info(void) {
	heap_node_p current_node = heap_nodes;

	size_t used_nodes_cnt = 0;
	size_t used_nodes_size = 0;
	size_t free_nodes_cnt = 0;
	size_t free_nodes_size = 0;

	while (current_node < heap_nodes + heap_nodes_count) {
		heap_ctl_p node_ctl = NODE_CTL(current_node);
		if (node_ctl->busy) {
			++used_nodes_cnt;
			used_nodes_size += node_ctl->size;
		} else {
			++free_nodes_cnt;
			free_nodes_size += node_ctl->size;
		}
		++current_node;
	}

	printf("===================== HEAP INFO =======================\n\r");
	printf("Start: %p\n\rSize: 0x%X\n\r", LOADER_HEAP_START, LOADER_HEAP_SIZE);
	printf("Used nodes: 0x%X in %d\n\r", used_nodes_size, used_nodes_cnt);
	printf("Free nodes: 0x%X in %d\n\r", free_nodes_size, free_nodes_cnt);
	printf("Unallocated space: 0x%X\n\r", LOADER_HEAP_SIZE - (used_nodes_size + free_nodes_size));
	printf("Max alloc size: 0x%X\n\r", max_alloc_size);
	printf("Min alloc size: 0x%X\n\r", min_alloc_size);
	printf("Total allocated: 0x%X (%d allocs)\n\r", total_allocated, total_allocs);
	printf("Total released: 0x%X (%d frees)\n\r", total_released, total_frees);
	printf("=======================================================\n\r");
}
