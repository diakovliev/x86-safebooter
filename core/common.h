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

static inline void outw(word_t port, word_t word) {
   __asm__ __volatile__ ("outw %1,%0" : : "dN" (port), "a"(word) );
}

static inline dword_t inl(word_t port) {
   dword_t ret;
   __asm__ __volatile__ ("inl %1, %0" : "=a" (ret) : "dN" (port));
   return ret;
}

static inline void outl(word_t port, dword_t dword) {
   __asm__ __volatile__ ("outl %1,%0" : : "dN" (port), "a"(dword) );
}

static inline void idle(void) {
	__asm__ __volatile__ ("pause");
}

static inline quad_t rdtsc() {
	quad_t x,y;
	__asm__ __volatile__ (
		"rdtsc\n"
		"movl %%eax,%0\n"
		"movl %%edx,%1\n"
		: "=r" (y), "=r" (x)
		:
		: "eax", "edx"
	);
	return (x << 32) | y;
}

#endif
