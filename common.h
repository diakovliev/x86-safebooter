#ifndef COMMON_HEADER
#define COMMON_HEADER

#include "loader_types.h"

static inline void outb(word_t port, byte_t byte) {
	asm volatile ("outb %1,%0" : : "dN" (port), "a"(byte) );
}

static inline byte_t inb(word_t port) {
   byte_t ret;
   asm volatile("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

static inline word_t inw(word_t port) {
   word_t ret;
   asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

#endif
