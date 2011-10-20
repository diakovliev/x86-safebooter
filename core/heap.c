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

#define CALCULATE_HEAP_STATISTIC

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

/* Current heap start */
static void *heap_start = 0;
static unsigned long heap_size = 0;

/* Pointer to nodes array */
static heap_node_p heap_nodes = 0;

/* Statistic */
#ifdef CALCULATE_HEAP_STATISTIC
static long max_alloc_size = 0;
static long min_alloc_size = 0;
static long total_allocated = 0;
static long total_released = 0;
static long total_allocs = 0;
static long total_frees = 0;
static long max_used_nodes_size = 0;
static long max_used_nodes_cnt = 0;
#endif

#define NODE_CTL(p) ((heap_ctl_p)(p->start - sizeof(heap_ctl_t)))

/* Initialize heap */
void heap_init(void *_heap_start, size_t _heap_size)
{
	heap_start			= _heap_start;
	heap_size			= _heap_size;
	heap_nodes			= (heap_node_p)(heap_start + heap_size);
	heap_nodes_count	= 0;
#ifdef CALCULATE_HEAP_STATISTIC
	max_alloc_size 		= 0;
	min_alloc_size 		= 0;
	total_allocated 	= 0;
	total_released 		= 0;
	total_allocs 		= 0;
	total_frees 		= 0;
	max_used_nodes_size	= 0;
	max_used_nodes_cnt	= 0;
#endif
}

/* find free nodes */
static heap_node_p get_free_node(size_t size) 
{
	heap_node_p current_node = heap_nodes;
	void *address = heap_start;
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

#ifdef CALCULATE_HEAP_STATISTIC
static inline void calculate_heap_usage(
	long *used_nodes_size,
	long *free_nodes_size,
	long *used_nodes_cnt,
	long *free_nodes_cnt
	) 
{
	heap_node_p current_node = heap_nodes;

	while (current_node < heap_nodes + heap_nodes_count) {
		heap_ctl_p node_ctl = NODE_CTL(current_node);
		if (node_ctl->busy) {
			*used_nodes_cnt	+= 1;
			*used_nodes_size	+= node_ctl->size;
		} else {
			*free_nodes_cnt	+= 1;
			*free_nodes_size	+= node_ctl->size;
		}
		++current_node;
	}
}
#endif

void *malloc(size_t size)
{
	heap_node_p node = get_free_node(size);
	if (node) {
		NODE_CTL(node)->busy = 1;

#ifdef CALCULATE_HEAP_STATISTIC
		/* collect stats */
		max_alloc_size = HEAP_MAX(max_alloc_size,NODE_CTL(node)->size);
		min_alloc_size = !min_alloc_size ? NODE_CTL(node)->size : HEAP_MIN(min_alloc_size,NODE_CTL(node)->size);
		total_allocated += NODE_CTL(node)->size;
		total_allocs += 1;

		long used_nodes_size = 0;
		long free_nodes_size = 0;
		long used_nodes_cnt = 0;
		long free_nodes_cnt = 0;

		calculate_heap_usage(
			&used_nodes_size,
			&free_nodes_size,
			&used_nodes_cnt,
			&free_nodes_cnt);
	
		if (used_nodes_size > max_used_nodes_size) {
			max_used_nodes_size	= used_nodes_size;
			max_used_nodes_cnt	= used_nodes_cnt;
		}
#endif

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
	if (current_node >= heap_nodes + heap_nodes_count) {
		printf("!!! Was tried to free unknown node at %p !!!\n\r", ptr);
		return;
	}

#ifdef CALCULATE_HEAP_STATISTIC
	/* collect stats */
	total_released += NODE_CTL(current_node)->size;
	total_frees += 1;
#endif

	NODE_CTL(current_node)->busy = 0;
}

void dump_heap_info(void) {

	printf("===================== HEAP INFO =======================\n\r");
#ifdef CALCULATE_HEAP_STATISTIC
	heap_node_p current_node = heap_nodes;

	long used_nodes_size = 0;
	long free_nodes_size = 0;
	long used_nodes_cnt = 0;
	long free_nodes_cnt = 0;

	calculate_heap_usage(
		&used_nodes_size,
		&free_nodes_size,
		&used_nodes_cnt,
		&free_nodes_cnt);

	printf("%ld bytes at %p\n\r", heap_size, heap_start);
	printf("Used nodes: %ld in %ld\n\r", used_nodes_size, used_nodes_cnt);
	printf("Free nodes: %ld in %ld\n\r", free_nodes_size, free_nodes_cnt);
	printf("Unallocated space: %ld\n\r", heap_size - (used_nodes_size + free_nodes_size));
	printf("Max alloc size: %ld\n\r", max_alloc_size);
	printf("Min alloc size: %ld\n\r", min_alloc_size);
	printf("Total allocated: %ld (%d allocs)\n\r", total_allocated, total_allocs);
	printf("Total released: %ld (%d frees)\n\r", total_released, total_frees);
	printf("Max used nodes: %ld in %ld\n\r", max_used_nodes_size, max_used_nodes_cnt);
	printf("===================== HEAP MAP ========================\n\r");
	while (current_node < heap_nodes + heap_nodes_count) {
		heap_ctl_p node_ctl = NODE_CTL(current_node);
		
		printf("%s %p %d\n\r",
			node_ctl->busy ? "*" : " ", 
			current_node->start, 
			node_ctl->size);

		++current_node;
	}
#endif
	printf("=======================================================\n\r");

}
