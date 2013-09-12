#ifndef DSA_BASE_HEADER
#define DSA_BASE_HEADER

#include "bch.h"

/* ALU byte length */
//#define DSA_SIZE	256
#define DSA_SIZE	(256+4)

#define SHA2_SIZE 160
#define SHA2_func sha1

static inline bch_p dsa_alloc(void) {
	return bch_alloc(DSA_SIZE);
}

static inline void dsa_free(bch_p ptr) {
	bch_free(ptr);
}

static inline bch_p dsa_from_ba(bch_data_p src, bch_size src_size) {
	return bch_rev(bch_from_ba(DSA_SIZE,src,src_size));
}

#endif /* DSA_BASE_HEADER */
