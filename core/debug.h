/*
 * debug.h
 *
 *  Created on: Aug 8, 2011
 *      Author: D.Iakovliev
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef __DEBUG__
#	ifndef DBG
#		define DBG(X) X
#	endif
#else
#	ifndef DBG
#		define DBG(X)
#	endif
#endif
#ifndef DBG_print
#	define DBG_print(...) DBG(printf(__VA_ARGS__))
#endif

#ifdef __HOST_COMPILE__

#include <stdlib.h>
#include <assert.h>

#else /* __HOST_COMPILE__ */

#include <common.h>

/* Assertion failed handler added to make possible to set break at assertion */
DBG(
static inline void on_assertion_failed(void) __attribute__((noreturn));
static inline void on_assertion_failed(void) {
	do { idle(); } while (1);
}
)
#define assert(x) DBG({if(!x){DBG_print("Assertion failed \"%s\" at %s:%d\n\r",#x,__FILE__,__LINE__);on_assertion_failed();}})

#endif /* __HOST_COMPILE__ */

#endif /* DEBUG_H_ */
