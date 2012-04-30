#include "heap.h"
#include <loader.h>

#include <stdio.h>
#include <string.h>
#include <debug.h>

#define IS_NOT_IN_HEAP(heap,ptr) (((void*)(ptr) < heap->start) || ((void*)(ptr) >= (void*)(heap->start + heap->size)))

/* As malloc and free are frequently used in "stack like" way it makes sence to perform 
 * looking nodes for allocation in direct order and for free in back order. 
 */

/* find free nodes */
static heap_node_p heap__get_free_node(heap_p heap, size_t size) 
{
	heap_node_p		prev_node = 0;
	heap_node_p 	current_node = heap->start;
	void 			*address = heap->start + sizeof(heap_node_t);

	do {			

		if (!current_node->busy && current_node->size >= size) {
			return current_node;
		}

		if (!current_node->start && !current_node->next) {
			if (address + size > heap->start + heap->size) {
				/* No memory */
				DBG_print("heap::get_free_node(%p): no memory to allocate %d bytes\n\r", heap, size);
				break;
			}

			current_node->start = address;
			current_node->raw_start = address;
			current_node->size = size;
			current_node->prev = prev_node;
			current_node->next = current_node->raw_start + current_node->size;
			/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible next
			 * allocation 
			 */
			memset(current_node->next, 0,  sizeof(heap_node_t));
			heap->last = current_node;
			return current_node;
		}

		address	= address + current_node->size + sizeof(heap_node_t);
		prev_node = current_node;
		current_node = current_node->next;

	} while (1);

	return 0;
}

/* find node by pointer */
static heap_node_p heap__find_node(heap_p heap, void *ptr) {

	heap_node_p 	current_node = heap->last;

	if (IS_NOT_IN_HEAP(heap,ptr)) {
		DBG_print("heap::find_node(%p): ptr %p is not in valid heap address\n\r", heap, ptr);
		return 0;
	}

	while (current_node && current_node->start != ptr) {
		current_node = current_node->prev;
		if (IS_NOT_IN_HEAP(heap,current_node)) {
			DBG_print("heap::find_node(%p): current_node %p is not in valid heap address\n\r", heap, current_node);
			return 0;
		}
	}

	return current_node;
}

void *heap__malloc(heap_p heap, size_t size)
{
	heap_node_p node = heap__get_free_node(heap, size);
	if (node) {

		node->busy = 1;
		return node->start;	
	}

	DBG_print("heap::heap_malloc(%p): allocation failed\n\r", heap);

	return 0;
}

void *heap__memalign(heap_p heap, size_t align, size_t size)
{
	/* allocate block with (size + align) size to guarantee enough block */
	heap_node_p node = heap__get_free_node(heap, size + align);
	if (node) {

		node->busy = 1;

		/* align pointer */
		if ((dword_t)node->start % align) {
			node->start = ((((dword_t)node->start / align) + 1) * align);
		}

		return node->start;	
	}

	DBG_print("heap::heap__memalign(%p): allocation failed\n\r", heap);

	return 0;

}

void heap__free(heap_p heap, void *ptr)
{
	heap_node_p node = heap__find_node(heap, ptr);
	if (!node) {
		DBG_print("heap::heap_free(%p): %p is unknown address to free\n\r", heap, ptr);
		return;
	}

	node->busy = 0;
	node->start = node->raw_start;
}

/* Initialize heap */
void heap__init(heap_p heap, void *start, size_t size)
{
	heap->start					= start;
	heap->size					= size;
	heap->first = heap->last	= heap->start;

	/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible first 
	 * allocation 
	 */
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

void *memalign(size_t align, size_t size)
{
	return heap__memalign(&global_heap, align, size);
}

void free(void *ptr)
{
	return heap__free(&global_heap, ptr);
}

void dump_heap_info(void) {
	puts("===================== HEAP INFO =======================\n\r");
	puts("=======================================================\n\r");
}
