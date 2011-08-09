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

#endif /* DEBUG_H_ */
