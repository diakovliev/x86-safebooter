#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <loader_types.h>
#include <stdio.h>

typedef struct image_block_s {

} image_block, *image_block_p;

typedef struct image_s {

} image, *image_p;

/* Load kernel image to memory */
byte_t image_load_raw(blk_istream_p s, dword_t image_start, dword_t whole_image_sectors);
byte_t image_load_sig(blk_istream_p s, dword_t image_start);

/* Boot */
void image_boot(loader_descriptor_p desc);

#endif /* IMAGE_HEADER */
