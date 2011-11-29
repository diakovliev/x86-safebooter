/*
 * bch.h
 *
 *  Created on: Aug 8, 2011
 *      Author: D.Iakovliev
 */

#ifndef BCH_H_
#define BCH_H_

#include <stdint.h>

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

/* Bit chains */
/********************************************************************************************************
 * BCH code
 ********************************************************************************************************/
#ifndef BCH_MAX
#define BCH_MAX(x,y) (x)>=(y)?(x):(y)
#endif
#ifndef BCH_MIN
#define BCH_MIN(x,y) (x)<(y)?(x):(y)
#endif
#ifndef BCH_ABS
#define BCH_ABS(x) (x)<0?(-x):(x)
#endif

/* Size type */
typedef uint16_t bch_size;
typedef uint8_t bch_data;
typedef uint16_t bch_data2x;
typedef bch_data *bch_data_p;

/* Bit chain type */
typedef struct bch_s {
	bch_size size;
	bch_data_p data;
} bch, *bch_p;

/* Random generator context */
typedef struct bch_random_s {
	/* Initialize generator */
	void (*init)(void);
	/* The function shall return random number from [ 0 ... max ]*/
	bch_data (*random)(bch_data max);
} bch_random, *bch_random_p;

/********************************************************************************************************
 * Forward
 ********************************************************************************************************/
#ifdef __DEBUG__
void bch__memory_usage();
#endif

void bch_hprint(const char *name, bch_p op);
void bch_print(const char *name, bch_p op);
bch_p bch_random_gen(bch_p dst, bch_p max, bch_random_p g);

bch_p bch_alloc(bch_size size);
void bch_va_free(bch_p ptr,...);
#define bch_free(...) bch_va_free(__VA_ARGS__,0)

bch_p bch_copy(bch_p dst, bch_p src);
bch_p bch_from_ba(bch_size dst_size, bch_data_p src, bch_size src_size);
bch_p bch_clone(bch_p src);

bch_p bch_rev(bch_p src);

int8_t bch_is_negative(bch_p op);
int8_t bch_is_zero(bch_p op);
int32_t bch_hexp(bch_p op);
int32_t bch_bexp(bch_p op);
int8_t bch_cmp(bch_p l, bch_p r);

bch_p bch_zero(bch_p op);
bch_p bch_one(bch_p op);
bch_p bch_abs(bch_p op);
bch_p bch_negate(bch_p op);
bch_p bch_byte_shl(bch_p dst, bch_size shift);
bch_p bch_byte_shr(bch_p dst, bch_size shift);
bch_p bch_bit_shl(bch_p dst, bch_size shift);
bch_p bch_bit_shr(bch_p dst, bch_size shift);
bch_p bch_set_bit(bch_p dst, uint32_t exp);

bch_p bch_add_s(bch_p dst, bch_data add);
bch_p bch_mul_s(bch_p dst, bch_data mul);

bch_p bch_add(bch_p dst, bch_p add);
bch_p bch_sub(bch_p dst, bch_p sub);
bch_p bch_mul(bch_p dst, bch_p mul);
bch_p bch_sqr(bch_p dst);
void bch_div_mod(bch_p r, bch_p m, bch_p divided, bch_p divider);
bch_p bch_div(bch_p dst, bch_p div);
bch_p bch_mod(bch_p dst, bch_p div);

bch_p bch_gcd(bch_p dst, bch_p a, bch_p b);
bch_p bch_gcdex(bch_p dst, bch_p a, bch_p b, bch_p x, bch_p y);
bch_p bch_gcdex_bin(bch_p dst, bch_p a, bch_p b, bch_p x, bch_p y);

bch_p bch_inverse(bch_p dst, bch_p a, bch_p n);
bch_p bch_inverse_bin(bch_p dst, bch_p a, bch_p n);

bch_p bch_pow(bch_p x,bch_p y);
bch_p bch_powmod(bch_p x,bch_p y,bch_p n);
bch_p bch_mulmod(bch_p x, bch_p y, bch_p n);

#endif /* BCH_H_ */
