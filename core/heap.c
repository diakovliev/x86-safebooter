#include "heap.h"
#include <loader.h>

/* Simple heap */

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

#define NODE_CTL(p) ((heap_ctl_p)(p->start - sizeof(heap_ctl_t)))

/* Initialize heap */
void heap_init() 
{
	heap_nodes			= (heap_node_p)(LOADER_HEAP_START + LOADER_HEAP_SIZE);
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
	if (address + size + sizeof(heap_ctl_t) < heap_nodes) {
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
		return node->start;	
	}
	return 0;
}

void free(void *ptr)
{
	heap_node_p current_node = heap_nodes;
	while (current_node->start != ptr && current_node < heap_nodes) {
		++current_node;
	}
	if (!current_node)
		return;

	NODE_CTL(current_node)->busy = 0;
}

