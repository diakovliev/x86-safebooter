#include "gmp.h"
#include <string.h>
#include <heap.h>
#include <drivers/console_iface.h>

/* TODO:
 *	- gmp_cmp should be rewritten
 *	- refactoring to replace (dst_op0,op1) code calls model by (dst,op0,op1) used now
 *	  this should decrease memory usage and make api simplest
 */

/********************************************************************************************************
 * GMP code
 ********************************************************************************************************/
#ifndef GMP_MAX
#define GMP_MAX(x,y) ((x>=y)?(x):(y))
#endif
#ifndef GMP_MIN
#define GMP_MIN(x,y) ((x<y)?(x):(y))
#endif
#ifndef GMP_ABS
#define GMP_ABS(x) ((x<0)?(-x):(x))
#endif
#ifndef GMP_SWAP
#define GMP_SWAP(a,b) a^=b, b^=a, a^=b;
#endif

#define GMP_NORMALIZE(op,i,base) \
	{ \
		int16_t v 				=	op->data[i]; \
		if (v >= 0) { \
			op->data[i]		=	v % base; \
			op->data[i+1]	+=	v / base; \
		} else { \
			op->data[i]		=	(base - GMP_ABS(v % base)) == base ? 0: (base - GMP_ABS(v % base)); \
			op->data[i+1]	-=	GMP_ABS(v) == base ? 1 : 1 + GMP_ABS(v/base); \
		} \
	}

#define GMP_NORMALIZE_TAIL(op,base) \
	{ \
		uint16_t i = 0; \
		int16_t d, m, v; \
		do { \
			v =	op->data[i]; \
			m = v % base; \
			d = v / base; \
			if ( m != v ) { \
				if (v >= 0) { \
					op->data[i++]		=	m; \
					op->data[i]			+=	d; \
				} else { \
					op->data[i++]		=	(base - GMP_ABS(m)) == base ? 0: (base - GMP_ABS(m)); \
					op->data[i]			-=	GMP_ABS(v) == base ? 1 : 1 + GMP_ABS(d); \
				} \
			} \
		} while ( m != v ); \
	}

/********************************************************************************************************
 * Tools
 ********************************************************************************************************/
inline uint8_t gmp_tools_is_negate(gmp_number_p op, uint16_t base) {
	return op->data[op->size-1] == base - 1;
}

int32_t gmp_tools_last_significant_digit(gmp_number_p op) {
	int32_t i = -1;
	for (i=op->size-1; i >= 0; --i)
		if (op->data[i])
			break;
	return i;
}

inline void gmp_tools_copy_number(gmp_number_p dst, gmp_number_p src) {
	memcpy(dst->data,src->data,sizeof(int16_t)*GMP_MIN(dst->size,src->size));
}

inline gmp_number_p gmp_tools_clone_number(gmp_number_p src) {
	gmp_number_p res = gmp_tools_alloc_number(src->size);
	gmp_tools_copy_number(res,src);
	return res;
}

gmp_number_p gmp_tools_alloc_number(uint16_t size) {
	gmp_number_p res = (gmp_number_p)malloc(sizeof(gmp_number_t));
	if (!res) return 0;
	res->size = size;
	/* !!! additional element for correct signing */
	res->data = (int16_t*)malloc(sizeof(int16_t)*(size+1));
	if (!res->data) {
		free(res);
		return 0;
	}
	memset(res->data, 0, sizeof(uint16_t)*(size+1));
	return res;
}

void gmp_tools_free_number(gmp_number_p ptr) {
	if (ptr->data) {
		memset(ptr->data, 0, sizeof(uint16_t)*ptr->size);
		free(ptr->data);
	}
	ptr->data = 0;
	ptr->size = 0;
	free(ptr);
}

void gmp_tools_dump_number(const char *fmt, gmp_number_p op, uint16_t base) {
	if (!gmp_tools_is_negate(op,base)) {
		int32_t i = gmp_tools_last_significant_digit(op);
		if (i < 0) {
			printf("0");
		} else
		for (; i >= 0; --i) {
			printf(fmt, op->data[i]);
		}
	} else {
		gmp_number_p tmp = gmp_tools_alloc_number(op->size);
		gmp_get_negate(tmp,op,base);
		int32_t i = gmp_tools_last_significant_digit(tmp);
		printf("-");
		if (i < 0) {
			printf("0");
		} else
		for (; i >= 0; --i) {
			printf(fmt, tmp->data[i]);
		}
		gmp_tools_free_number(tmp);
	}
}

inline void gmp_tools_create_exp(gmp_number_p res, uint16_t exp, int16_t value) {
	memset(res->data, 0, sizeof(int16_t)*res->size);
	res->data[exp] = value;
}

inline void gmp_tools_shl(gmp_number_p res, gmp_number_p op, uint16_t exp) {
	memset(res->data, 0, sizeof(int16_t)*res->size);
	memcpy(res->data+exp, op->data, GMP_MIN((op->size),(res->size-exp))*sizeof(int16_t));
}

inline void gmp_tools_create_zero(gmp_number_p op) {
	memset(op->data, 0, sizeof(int16_t)*op->size);
}

uint64_t gmp_tools_get_small(gmp_number_p op, uint16_t base) {
	uint64_t res;
	uint16_t i,b = 1;
	for (i = 0; i < op->size; i++) {
		res += op->data[i] * b;
		b *= base;
	}
	return res;
}

void gmp_tools_reverse(gmp_number_p op, uint16_t size) {
	uint16_t i;
	int16_t c;
	for (i = 0; i < size/2; i++) {
		c = op->data[size-1-i];
		op->data[size-1-i] = op->data[i];
		op->data[i] = c;
	}
}

int16_t gmp_tools_is_zero(gmp_number_p op) {
	uint16_t i;
	int16_t c = 0;
	for (i = 0; i < op->size; i++) {
		c |= op->data[i];
		if (c) break;
	}
	return !c;
}

/********************************************************************************************************
 * Code
 ********************************************************************************************************/
/*
 * results:
 *  op0 > op1 --- returns value > 0
 *  op0 < op1 --- returns value < 0
 *  op0 == op1 --- returns value  0
 */
int8_t gmp_cmp(gmp_number_p op0_, gmp_number_p op1_, uint16_t base) {
	uint8_t result = 0, invert_result = 0;
	int32_t i;
	gmp_number_p op0 = 0, op1 = 0;

	/* sign */
	if (gmp_tools_is_negate(op0_,base) != gmp_tools_is_negate(op1_,base))
		return gmp_tools_is_negate(op0_,base) - gmp_tools_is_negate(op1_,base);

	/* needs inversion? */
	if (gmp_tools_is_negate(op0_,base)) {
		op0 = gmp_tools_alloc_number(op0_->size);
		gmp_get_abs(op0,op0_,base);
		op1 = gmp_tools_alloc_number(op1_->size);
		gmp_get_abs(op1,op1_,base);
		invert_result = 1;
	} else {
		op0 = op0_;
		op1 = op1_;
	}

	/* Compare with zero */
	uint8_t op0_z = gmp_tools_is_zero(op0);
	uint8_t op1_z = gmp_tools_is_zero(op1);
	if (op0_z && !op1_z)
		return invert_result ? 1 : -1;
	if (!op0_z && op1_z)
		return invert_result ? -1 : 1;
	if (op0_z && op1_z)
		return 0;

	/* Comare by module */
	int32_t op0_s = gmp_tools_last_significant_digit(op0);
	int32_t op1_s = gmp_tools_last_significant_digit(op1);
	if (op0_s != op1_s) {
		result = op0_s - op1_s > 0 ? 1 : -1;
		if (invert_result)
			result = result == -1 ? 1 : -1;
	}
	for(i=GMP_MAX(op0_s,op1_s); i >= 0; --i)
		if (op0->data[i] - op1->data[i]) {
			result = op0->data[i] - op1->data[i] > 0 ? 1: -1;
			if (invert_result)
				result = result == -1 ? 1 : -1;
			break;
		}
	if(invert_result) {
		gmp_tools_free_number(op0);
		gmp_tools_free_number(op1);
	}
	return result;
}

void gmp_add(gmp_number_p res, gmp_number_p left, gmp_number_p right, uint16_t base) {
	uint16_t i;
	gmp_tools_create_zero(res);

	for (i = 0; i < GMP_MAX(left->size, right->size); ++i) {
		res->data[i] += left->data[i] + right->data[i];
		GMP_NORMALIZE(res,i,base);
	}
}

void gmp_add_s(gmp_number_p res, gmp_number_p left, int16_t right, uint16_t base) {
	gmp_tools_create_zero(res);

	gmp_tools_copy_number(res,left);
	res->data[0] = left->data[0] + right;
	GMP_NORMALIZE_TAIL(res,base);
}

void gmp_get_negate(gmp_number_p res, gmp_number_p op, uint16_t base) {
	uint16_t i;
	gmp_tools_create_zero(res);
	for (i = 0; i < op->size; ++i) {
		res->data[i] += (base - 1) - op->data[i];
	}

	gmp_number_p tmp = gmp_tools_clone_number(res);
	gmp_add_s(res,tmp,1,base);
	gmp_tools_free_number(tmp);
}

inline void gmp_get_abs(gmp_number_p res, gmp_number_p op, uint16_t base) {
	if (gmp_tools_is_negate(op,base)) {
		gmp_get_negate(res,op,base);
	} else {
		gmp_tools_copy_number(res,op);
	}
}

inline void gmp_sub(gmp_number_p res, gmp_number_p left_, gmp_number_p right_, uint16_t base) {
	gmp_number_p right = gmp_tools_alloc_number(right_->size);
	gmp_get_negate(right,right_,base);
	gmp_add(res,left_,right,base);
	gmp_tools_free_number(right);
}

inline void gmp_sub_s(gmp_number_p res, gmp_number_p left, int16_t right, uint16_t base) {
	gmp_add_s(res,left,-right,base);
}

void gmp_mul_s(gmp_number_p res, gmp_number_p left_, int16_t right, uint16_t base) {
	uint16_t i;
	int16_t v;
	gmp_tools_create_zero(res);

	gmp_number_p left = 0;
	uint8_t negate = gmp_tools_is_negate(left_,base) ^ (right >= 0 ? 0 : 1);
	if(negate) {
		left = gmp_tools_alloc_number(left_->size);
		gmp_get_abs(left,left_,base);
	} else {
		left = left_;
	}

	for (i = 0; i < left->size; ++i) {
		if (res->size>i) {
			res->data[i] += left->data[i] * GMP_ABS(right);
			if (res->size>i+1) {
				v = res->data[i];
				res->data[i]	=	v % base;
				res->data[i+1]	+=	v / base;
			}
		}
	}

	if (negate) {
		gmp_number_p res_ = gmp_tools_clone_number(res);
		gmp_get_negate(res,res_,base);
		gmp_tools_free_number(res_);
		gmp_tools_free_number(left);
	}
}

void gmp_mul(gmp_number_p res, gmp_number_p left_, gmp_number_p right_, uint16_t base) {
	uint16_t i,j;
	int16_t v;
	gmp_tools_create_zero(res);

	uint8_t negate = gmp_tools_is_negate(left_,base) ^ gmp_tools_is_negate(right_,base);

	gmp_number_p left = gmp_tools_alloc_number(left_->size);
	gmp_get_abs(left,left_,base);
	gmp_number_p right = gmp_tools_alloc_number(right_->size);
	gmp_get_abs(right,right_,base);

	for (i = 0; i < left->size; ++i) {
		for(j = 0; j < right->size; ++j) {
			if (res->size>i+j) {
				res->data[i+j] 		+= left->data[i] * right->data[j];
				if (res->size>i+j+1) {
					v = res->data[i+j];
					res->data[i+j]		=	v % base;
					res->data[i+j+1]	+=	v / base;
				}
			}
		}
	}

	if (negate) {
		gmp_number_p res_ = gmp_tools_clone_number(res);
		gmp_get_negate(res,res_,base);
		gmp_tools_free_number(res_);
	}

	gmp_tools_free_number(left);
	gmp_tools_free_number(right);
}

void gmp_div_s_f(gmp_number_p res, uint16_t *mod, gmp_number_p divided_, int16_t divider, uint16_t base) {
	int16_t i,j=0;
	uint8_t local_res = 0;

	if (res) gmp_tools_create_zero(res);

	if(gmp_tools_is_zero(divided_) || !divider || (!res && !mod))
		return;

	if (!res) {
		res = gmp_tools_alloc_number(divided_->size);
		local_res = 1;
	}

	uint8_t negate = gmp_tools_is_negate(divided_,base) ^ (divider >= 0 ? 0 : 1);
	gmp_number_p divided = gmp_tools_alloc_number(divided_->size);
	gmp_get_abs(divided,divided_,base);

	int16_t v;
	i = gmp_tools_last_significant_digit(divided);
	for (; i >= 0; --i) {
		v				= 	res->data[j];
		res->data[j++] 	= 	(divided->data[i]+(v*base))/(GMP_ABS(divider)*base);
		res->data[j] 	= 	(divided->data[i]+(v*base))%(GMP_ABS(divider)*base);
	}

	gmp_tools_reverse(res,j);

	if (negate) {
		gmp_number_p res_ = gmp_tools_clone_number(res);
		gmp_get_negate(res,res_,base);
		gmp_tools_free_number(res_);
	}

	gmp_tools_free_number(divided);

	if (mod) *mod = v;

	if (local_res)  {
		gmp_tools_free_number(res);
	}
}

inline void gmp_div_s(gmp_number_p res, gmp_number_p divided, int16_t divider, uint16_t base) {
	gmp_div_s_f(res,0,divided,divider,base);
}

inline uint16_t gmp_mod_s(gmp_number_p divided, int16_t divider, uint16_t base) {
	uint16_t res = 0;
	gmp_div_s_f(0,&res,divided,divider,base);
	return res;
}

void gmp_div_f(gmp_number_p res, gmp_number_p mod, gmp_number_p divided_, gmp_number_p divider_, uint16_t base) {
	
	uint8_t local_mod = 0;

	if (res) gmp_tools_create_zero(res);
	if (mod) gmp_tools_create_zero(mod);

	if (gmp_tools_is_zero(divider_) || gmp_tools_is_zero(divided_) || (!res && !mod))
		return;

	/* divider > divided */
	if ( 0 > gmp_cmp(divided_,divider_,base) ) {
		gmp_sub(mod,divider_,divided_,base);
		return;
	}

	gmp_tools_create_zero(mod);

	if (!mod) {
		mod = gmp_tools_alloc_number(divided_->size);
		local_mod = 1;
	}

	uint8_t negate = gmp_tools_is_negate(divided_,base) ^ gmp_tools_is_negate(divider_,base);
	gmp_number_p divided = gmp_tools_alloc_number(divided_->size);
	gmp_get_abs(divided,divided_,base);
	gmp_number_p divider = gmp_tools_alloc_number(divider_->size);
	gmp_get_abs(divider,divider_,base);

	uint16_t divided_exp = gmp_tools_last_significant_digit(divided)+1;
	uint16_t divider_exp = gmp_tools_last_significant_digit(divider)+1;

	int32_t exp = (divided_exp-divider_exp);
	
	gmp_number_p e		= gmp_tools_alloc_number(divided->size+1);
	gmp_number_p s 		= gmp_tools_alloc_number(divided->size+1);
	gmp_number_p tmp 	= gmp_tools_alloc_number(divided->size+1);

	gmp_tools_copy_number(mod, divided);

	int16_t i;
	int8_t c;
	while(gmp_cmp(mod, divider,base) >= 0) {

		if (exp >= 0) {
			gmp_tools_shl(s, divider, exp--);
		}
		else break;

		i = 1;
		gmp_tools_create_zero(e);
		do {
			gmp_add(tmp,e,s,base);
			gmp_tools_copy_number(e,tmp);
			c = gmp_cmp(e,mod,base);
		} while (c < 0 && i++ < base);
		i = c > 0 ? i-1 : i;
		if (c) {
			gmp_sub(tmp,e,s,base);						
			gmp_tools_copy_number(e,tmp);
		}

		gmp_tools_copy_number(s,e);
		if (gmp_cmp(mod,s,base) >= 0) {
			gmp_tools_create_exp(e,exp+1,i); 

			if (res) {
				gmp_add(tmp,res,e,base);
				gmp_tools_copy_number(res,tmp);
			}

			gmp_sub(tmp,mod,s,base);
			gmp_tools_copy_number(mod,tmp);
		}
	}

	if (negate) {
		gmp_number_p res_ = gmp_tools_clone_number(res);
		gmp_get_negate(res,res_,base);
		gmp_tools_free_number(res_);
	}

	gmp_tools_free_number(divided);
	gmp_tools_free_number(divider);

	gmp_tools_free_number(s);
	gmp_tools_free_number(e);
	gmp_tools_free_number(tmp);

	if (local_mod) gmp_tools_free_number(mod);
}

inline void gmp_div(gmp_number_p res, gmp_number_p divided, gmp_number_p divider, uint16_t base) {
	gmp_div_f(res, 0, divided, divider, base);
}

inline void gmp_mod(gmp_number_p res, gmp_number_p divided, gmp_number_p divider, uint16_t base) {
	gmp_div_f(0, res, divided, divider, base);
}

void gmp_pow_s(gmp_number_p res, gmp_number_p op, uint16_t power, uint16_t base) {
	gmp_number_p tmp 		= gmp_tools_alloc_number(res->size);
	gmp_number_p work_op	= gmp_tools_alloc_number(op->size);
	gmp_tools_copy_number(work_op, op);

	gmp_tools_create_exp(res, 0, 1);
	while (power) {
		if (power & 1) {
			gmp_mul(tmp, res, work_op, base);
			gmp_tools_copy_number(res, tmp);
		}		
		gmp_mul(tmp, work_op, work_op, base);		
		gmp_tools_copy_number(work_op, tmp);
		power >>= 1;
	}

	gmp_tools_free_number(tmp);
	gmp_tools_free_number(work_op);
}

void gmp_pow(gmp_number_p res, gmp_number_p a, gmp_number_p power, uint16_t base) {
/*
long powmod(long a, long k, long n)
{
  long b=1;

  while (k) {
    if (k%2==0) {
      k /= 2;
      a *= a;
      }
    else {
      k--;
      b *= a;
      }
  }
  return b;
}
*/
	gmp_number_p b		= gmp_tools_alloc_number(res->size);
	gmp_number_p k		= gmp_tools_clone_number(power);
	gmp_number_p tmp	= gmp_tools_alloc_number(res->size);

	gmp_tools_create_exp(b,0,1);
	while (!gmp_tools_is_zero(k)) {
		if (!gmp_mod_s(k,2,base)) {
			gmp_div_s(tmp,k,2,base);
			gmp_tools_copy_number(k,tmp);
			gmp_mul(tmp,a,a,base);
			gmp_tools_copy_number(a,tmp);
		}
		else {
			gmp_sub_s(tmp,k,1,base);
			gmp_tools_copy_number(k,tmp);
			gmp_mul(tmp,b,a,base);
			gmp_tools_copy_number(b,tmp);
		}
	}

	gmp_tools_free_number(b);
	gmp_tools_free_number(k);
	gmp_tools_free_number(tmp);
}

/* a^n mod b === [(a%b)^n mod b] mod b */
#define USE_MOD_OPTIMISATION

void gmp_powmod(gmp_number_p res, gmp_number_p a_, gmp_number_p power, gmp_number_p mod, uint16_t base) {
/*
long powmod(long a, long k, long n)
{
  long b=1;

  while (k) {
    if (k%2==0) {
      k /= 2;
      a = (a*a)%n;
      }
    else {
      k--;
      b = (b*a)%n;
      }
  }
  return b;
}
*/
	gmp_number_p a		= gmp_tools_clone_number(a_);
	gmp_number_p b		= gmp_tools_alloc_number(res->size);
	gmp_number_p k		= gmp_tools_clone_number(power);
	gmp_number_p tmp	= gmp_tools_alloc_number(res->size);

#ifdef USE_MOD_OPTIMISATION
	gmp_mod(tmp,a_,mod,base);
	gmp_tools_copy_number(a,tmp);
#endif/*USE_MOD_OPTIMISATION*/

	gmp_tools_create_exp(b,0,1);
	while (!gmp_tools_is_zero(k)) {
		if (!gmp_mod_s(k,2,base)) {
			gmp_div_s(tmp,k,2,base);
			gmp_tools_copy_number(k,tmp);
			gmp_mul(tmp,a,a,base);
			gmp_mod(a,tmp,mod,base);
		}
		else {
			gmp_sub_s(tmp,k,1,base);
			gmp_tools_copy_number(k,tmp);
			gmp_mul(tmp,b,a,base);
			gmp_mod(b,tmp,mod,base);
		}
	}

#ifdef USE_MOD_OPTIMISATION
	gmp_mod(tmp,b,mod,base);
	gmp_tools_copy_number(b,tmp);
#endif/*USE_MOD_OPTIMISATION*/

	gmp_tools_copy_number(res,b);

	gmp_tools_free_number(a);
	gmp_tools_free_number(b);
	gmp_tools_free_number(k);
	gmp_tools_free_number(tmp);
}


void gmp_gcd(gmp_number_p res, gmp_number_p a_, gmp_number_p b_, uint16_t base) {
	gmp_tools_create_zero(res);

	gmp_number_p mod	= gmp_tools_alloc_number(res->size);
	gmp_number_p a		= gmp_tools_clone_number(a_);
	gmp_number_p b		= gmp_tools_clone_number(b_);

	while (!gmp_tools_is_zero(a) && !gmp_tools_is_zero(b)) {
		if (gmp_cmp(a,b,base) > 0) {
			gmp_mod(mod,a,b,base);
			gmp_tools_copy_number(a,mod);
		} else {
			gmp_mod(mod,b,a,base);
			gmp_tools_copy_number(b,mod);
		}
	}

	gmp_add(res,a,b,base);

	gmp_tools_free_number(mod);
	gmp_tools_free_number(a);
	gmp_tools_free_number(b);
}

void gmp_gcdex(gmp_number_p d, gmp_number_p a_, gmp_number_p b_, gmp_number_p x, gmp_number_p y, uint16_t base) {

	gmp_number_p	q	= gmp_tools_alloc_number(d->size), 
					r	= gmp_tools_alloc_number(d->size), 
					tmp	= gmp_tools_alloc_number(d->size), 
					x1 	= gmp_tools_alloc_number(x->size),
					x2 	= gmp_tools_alloc_number(x->size), 
					y1 	= gmp_tools_alloc_number(y->size),
					y2 	= gmp_tools_alloc_number(y->size),
					zero= gmp_tools_alloc_number(1),
					a	= gmp_tools_clone_number(a_),
					b	= gmp_tools_clone_number(b_);

	if (gmp_tools_is_zero(b)) {
		gmp_tools_copy_number(d,a);
		gmp_tools_create_exp(x,0,1);
		gmp_tools_create_zero(y);
		return;
	}

	gmp_tools_create_exp(x2,0,1);
	gmp_tools_create_zero(x1);
	gmp_tools_create_zero(y2);
	gmp_tools_create_exp(y1,0,1);	

	while (gmp_cmp(b,zero,base) > 0) {
		gmp_div_f(q,r,a,b,base);

		gmp_mul(tmp,q,x1,base);
		gmp_sub(x,x2,tmp,base);
		gmp_mul(tmp,q,y1,base);
		gmp_sub(y,y2,tmp,base);

		gmp_tools_copy_number(a,b);
		gmp_tools_copy_number(b,r);

		gmp_tools_copy_number(x2,x1);
		gmp_tools_copy_number(x1,x);
		gmp_tools_copy_number(y2,y1);
		gmp_tools_copy_number(y1,y);
	}

	gmp_tools_copy_number(d,a);
	gmp_tools_copy_number(x,x2);
	gmp_tools_copy_number(y,y2);

	gmp_tools_free_number(q); 
	gmp_tools_free_number(r); 
	gmp_tools_free_number(tmp);
	gmp_tools_free_number(x1);
	gmp_tools_free_number(x2); 
	gmp_tools_free_number(y1);
	gmp_tools_free_number(y2);
	gmp_tools_free_number(zero);
	gmp_tools_free_number(a);
	gmp_tools_free_number(b);
}

uint8_t gmp_inverse(gmp_number_p res, gmp_number_p a, gmp_number_p n, uint16_t base)
{
	uint8_t result = 0;
	gmp_number_p tmp = gmp_tools_alloc_number(res->size);
	gmp_number_p x = gmp_tools_alloc_number(res->size);
	gmp_number_p y = gmp_tools_alloc_number(res->size);

	gmp_tools_create_exp(tmp,0,1);

	gmp_gcdex(res, a, n, x, y, base);
	if (gmp_cmp(res,tmp,base) == 0) {
		gmp_tools_copy_number(res,x);
		result = 1;
	}

	gmp_tools_free_number(x);
	gmp_tools_free_number(y);
	gmp_tools_free_number(tmp);

	return result;
}

inline uint64_t gmp_tools_rdtsc() {
	uint64_t x,y;
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

uint32_t gmp_tools_random(uint32_t N) {
	return  (uint32_t)gmp_tools_rdtsc() % N+1;
}

/********************************************************************************************************
 * Test code
 ********************************************************************************************************/

#if 0
void test_gmp(void) {

	uint16_t size = 4;
	uint16_t alloc_size = size*2;
	uint16_t sz_div = 2;
	uint16_t base = 10;

	gmp_number_p op0 = gmp_tools_alloc_number(alloc_size);
	gmp_number_p op1 = gmp_tools_alloc_number(alloc_size);
	gmp_number_p res = gmp_tools_alloc_number(alloc_size);
	gmp_number_p div = gmp_tools_alloc_number(alloc_size);
	gmp_number_p mod = gmp_tools_alloc_number(alloc_size);

	op0->data[0] = 5;
	op1->data[0] = 3;
	mod->data[0] = 3;

	gmp_powmod(res, op0, op1, mod, base);
	gmp_tools_dump_number("%X", op0, base);
	printf("^");
	gmp_tools_dump_number("%X", op1, base);
	printf(" mod ");
	gmp_tools_dump_number("%X", mod, base);
	printf(" === ");
	gmp_tools_dump_number("%X", res, base);
	printf("\n\r");

	gmp_tools_free_number(op0);
	gmp_tools_free_number(op1);
	gmp_tools_free_number(res);
	gmp_tools_free_number(div);
	gmp_tools_free_number(mod);

	return 0;
}
#endif
