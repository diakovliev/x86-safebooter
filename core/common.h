#ifndef COMMON_HEADER
#define COMMON_HEADER

#include "loader_types.h"

static inline void outb(word_t port, byte_t byte) {
	__asm__ __volatile__ ("outb %1,%0" : : "dN" (port), "a"(byte) );
}

static inline byte_t inb(word_t port) {
   byte_t ret;
   __asm__ __volatile__ ("inb %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

static inline word_t inw(word_t port) {
   word_t ret;
   __asm__ __volatile__ ("inw %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

static inline void idle(void) {
	__asm__ __volatile__ ("nop;pause");
}

#endif
