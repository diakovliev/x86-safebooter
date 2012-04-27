#include "heap.h"
#include <loader.h>

#include <stdio.h>
#include <string.h>
#include <debug.h>

/* Simple heap */

/* Heap item */
typedef struct heap_node_s {
	void  *start;
	size_t size;
	uint8_t busy;
	struct heap_node_s *next;
} heap_node_t, *heap_node_p;

/* find free nodes */
static heap_node_p heap__get_free_node(heap_p heap, size_t size) 
{
	heap_node_p 	current_node = heap->start;
	void 			*address = heap->start + sizeof(heap_node_t);

	do {			

		if (!current_node->busy && current_node->size >= size) {
			return current_node;
		}

		if (!current_node->start && !current_node->next) {
			if ((size_t)address + size > (size_t)heap->start + heap->size) {
				/* No memory */
				DBG_print("heap::get_free_node(%p): no memory to allocate %d bytes\n\r", heap, size);
				break;
			}

			current_node->start = address;
			current_node->size = size;
			current_node->next = (size_t)current_node->start + current_node->size;
/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible next
 * allocation 
 * */
			memset(current_node->next, 0,  sizeof(heap_node_t));
			return current_node;
		}

		address	= (size_t)address + current_node->size + sizeof(heap_node_t);
		current_node = current_node->next;

	} while (1);

	return 0;
}

/* find node by pointer */
static heap_node_p heap__find_node(heap_p heap, void *ptr) {

	heap_node_p 	current_node = heap->start;

	if (ptr < heap->start || ptr >= (size_t)heap->start + heap->size) {
		DBG_print("heap::find_node(%p): ptr %p is not in valid heap address\n\r", heap, ptr);
		return 0;
	}

	do {			
		current_node	= current_node->next;
		if (current_node < heap->start || current_node >= (size_t)heap->start + heap->size) {
			DBG_print("heap::find_node(%p): current_node %p is not in valid heap address\n\r", heap, current_node);
			return 0;
		}

	} while (current_node && current_node->start != ptr);

	return current_node;
}

void *heap__malloc(heap_p heap, size_t size)
{
	heap_node_p node = heap__get_free_node(heap, size);
	if (node) {
		node->busy = 1;

		return node->start;	
	}

	DBG_print("heap::heap_malloc(%p): Allocation failed\n\r", heap);

	return 0;
}

void heap__free(heap_p heap, void *ptr)
{
	heap_node_p current_node = heap__find_node(heap, ptr);
	if (!current_node) {
		DBG_print("heap::heap_free(%p): %p is unknown address to free\n\r", heap, ptr);
		return;
	}

	current_node->busy = 0;
}

/* Initialize heap */
void heap__init(heap_p heap, void *start, size_t size)
{
	heap->start			= start;
	heap->size			= size;

/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible first 
 * allocation 
 * */
	memset(heap->start, 0,  sizeof(heap_node_t));
}

/***************************************************************************/
static heap_t global_heap = {0,0};

void heap_init(void *start, size_t size)
{
	heap__init(&global_heap, start, size);
}

void *malloc(size_t size)
{
	return heap__malloc(&global_heap, size);
}

void free(void *ptr)
{
	return heap__free(&global_heap, ptr);
}

void dump_heap_info(void) {
	puts("===================== HEAP INFO =======================\n\r");
	puts("=======================================================\n\r");
}
