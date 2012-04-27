#ifndef IMAGE_HEADER
#define IMAGE_HEADER

#include <loader_types.h>
#include <stdio.h>

/* Load kernel image to memory */
#ifdef CONFIG_RAW_IMAGES_ENABLED
byte_t image_load_raw(blk_iostream_p s, dword_t image_start, dword_t whole_image_sectors);
#endif/*CONFIG_RAW_IMAGES_ENABLED*/

byte_t image_load_sig(blk_iostream_p s, dword_t image_start);

/* Boot */
void image_boot(loader_descriptor_p desc);

#endif /* IMAGE_HEADER */
