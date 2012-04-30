#ifndef LOADER_TYPES_HEADER
#define LOADER_TYPES_HEADER

#include <stdint.h> /* uintXX_t */

/* Common types */
typedef uint8_t byte_t;
typedef byte_t *byte_p;
typedef uint16_t word_t;
typedef word_t *word_p;
typedef uint32_t dword_t;
typedef dword_t *dword_p;
typedef uint64_t quad_t;
typedef quad_t *quad_p;
typedef uint32_t size_t;

/* Loader descriptor */
#pragma pack(push,1)
typedef struct loader_descriptor_s {
	word_t magic;
	byte_t version[3];
	word_t loader_sectors_count;
	word_t unused;
	word_t loader_descriptor_sectors_count;
} loader_descriptor_t;
#pragma pack(pop)
typedef struct loader_descriptor_s *loader_descriptor_p;

#define LOADER_ENV(LOADER_DESCRIPTOR_POINTER) \
	(char*)((void*)LOADER_DESCRIPTOR_POINTER+DISK_SECTOR_SIZE)

#endif//LOADER_TYPES_HEADER
