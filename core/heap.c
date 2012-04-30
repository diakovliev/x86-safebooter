#include <loader.h>

#include <stdio.h>
#include <string.h>
#include <debug.h>

#define DEBUG_HEAP

#include "heap.h"

#define IS_NOT_IN_HEAP(heap,ptr) (((void*)(ptr) < heap->start) || ((void*)(ptr) >= (void*)(heap->start + heap->size)))

#ifdef DEBUG_HEAP
#define HEAP_CHECK_NODE(x) assert(((x)->magic_node_start == MAGIC_NODE_START) && ((x)->magic_node_end == MAGIC_NODE_END))
#else
#define HEAP_CHECK_NODE(x)
#endif


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
			HEAP_CHECK_NODE(current_node);
			return current_node;
		}

		if (!current_node->start && !current_node->next) {
			if (address + size > heap->start + heap->size) {
				/* No memory */
				DBG_print("heap::get_free_node(%p): no memory to allocate %d bytes\n\r", heap, size);
				break;
			}

#ifdef DEBUG_HEAP
			current_node->magic_node_start = MAGIC_NODE_START;
#endif
			current_node->start = address;
			current_node->raw_start = address;
			current_node->size = size;
			current_node->prev = prev_node;
			current_node->next = current_node->raw_start + current_node->size;
			/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible next
			 * allocation 
			 */
#ifdef DEBUG_HEAP
			current_node->magic_node_end = MAGIC_NODE_END;
#endif
			memset(current_node->next, 0,  sizeof(heap_node_t));
			heap->last = current_node;

			HEAP_CHECK_NODE(current_node);
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

	HEAP_CHECK_NODE(current_node);

	if (IS_NOT_IN_HEAP(heap,ptr)) {
		DBG_print("heap::find_node(%p): ptr %p is not in valid heap address\n\r", heap, ptr);
		return 0;
	}

	while (current_node && current_node->start != ptr) {
		current_node = current_node->prev;

		HEAP_CHECK_NODE(current_node);

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

		HEAP_CHECK_NODE(node);

		node->busy = 1;

#ifdef DEBUG_HEAP
		++heap->busy_nodes_count;
		++heap->alloc_count;
		heap->allocated += node->size;
#endif

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

		HEAP_CHECK_NODE(node);

		node->busy = 1;

#ifdef DEBUG_HEAP
		++heap->busy_nodes_count;
		++heap->alloc_count;
		heap->allocated += node->size;
#endif

		/* align pointer */
		if ((dword_t)node->start % align) {
			node->start = ((((dword_t)node->start / align) + 1) * align);
		}

		return node->start;	
	}

	DBG_print("heap::heap__memalign(%p): allocation failed\n\r", heap);

	return 0;

}

size_t heap__malloc_usable_size(heap_p heap, void *ptr) 
{
	size_t res = 0;

	heap_node_p node = heap__find_node(heap, ptr);
	if (node) {

		HEAP_CHECK_NODE(node);

		res = node->size;
	}

	return res;
}

void heap__free(heap_p heap, void *ptr)
{
	heap_node_p node = heap__find_node(heap, ptr);
	if (!node) {
		DBG_print("heap::heap_free(%p): %p is unknown address to free\n\r", heap, ptr);
		return;
	}

	HEAP_CHECK_NODE(node);

	node->busy = 0;
	node->start = node->raw_start;

#ifdef DEBUG_HEAP
	--heap->busy_nodes_count;
	++heap->free_nodes_count;
	++heap->free_count;
	heap->released += node->size;
#endif

}

/* Initialize heap */
void heap__init(heap_p heap, void *start, size_t size)
{
	heap->start					= start;
	heap->size					= size;
	heap->first = heap->last	= heap->start;

#ifdef DEBUG_HEAP
	heap->busy_nodes_count = 0;
	heap->alloc_count = 0;
	heap->allocated = 0;
	heap->free_nodes_count = 0;
	heap->free_count = 0;
	heap->released = 0;
#endif

	/* fill start sizeof(node_ctl_t) + sizeof(heap_node_p) by zero to make possible first 
	 * allocation 
	 */
	memset(heap->start, 0,  sizeof(heap_node_t));
}

/***************************************************************************/
static heap_t global_heap;

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

size_t malloc_usable_size(void *ptr)
{
	return heap__malloc_usable_size(&global_heap, ptr);
}

void dump_heap_info(void) {
	puts("===================== HEAP INFO =======================\n\r");
	printf("HEAP: %p\n\r", &global_heap);
	printf("\tstart: %p\n\r", global_heap.start);
	printf("\tsize: %d bytes\n\r", global_heap.size);
#ifdef DEBUG_HEAP
	printf("\tbusy nodes count: %d\n\r", global_heap.busy_nodes_count);
	printf("\tallocatins count: %d\n\r", global_heap.alloc_count);
	printf("\tallocated: %d bytes\n\r", global_heap.allocated);
	printf("\tfree nodes count: %d\n\r", global_heap.free_nodes_count);
	printf("\tfrees count: %d\n\r", global_heap.free_count);
	printf("\treleased: %d bytes\n\r", global_heap.released);
#else
	puts("\tDEBUG_HEAP is disabled\n\r");
#endif
	puts("=======================================================\n\r");
}
