#ifndef LOADER_TYPES_HEADER
#define LOADER_TYPES_HEADER

/* Common types */
typedef unsigned char byte_t;
typedef byte_t *byte_p;
typedef unsigned short word_t;
typedef word_t *word_p;
typedef unsigned int dword_t;
typedef dword_t *dword_p;

/* Loader descriptor */
typedef struct loader_descriptor_s {
	word_t magic;
	byte_t version[3];
	byte_t loader_sectors_count;
	byte_t loader_env[512-5]; // sector_size - 3 previous fields
} loader_descriptor_t;

#endif//LOADER_TYPES_HEADER
