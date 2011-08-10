#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <loader_types.h>

typedef struct image_block_s {

} image_block, *image_block_p;

typedef struct image_s {

} image, *image_p;

/* Load kernel image to memory */
extern byte_t image_load(word_t bus, byte_t drive, loader_descriptor_p desc);
extern void image_boot(loader_descriptor_p desc);

#endif /* IMAGE_HEADER */
