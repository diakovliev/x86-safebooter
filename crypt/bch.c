/*
 * bch.c
 *
 *  Created on: Aug 8, 2011
 *      Author: D.Iakovliev
 */

#include "bch.h"

#ifdef __HOST_COMPILE__
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#else
#include <drivers/console_iface.h>
#define assert(x)
#endif

#include <string.h>

/********************************************************************************************************
 * Tools
 ********************************************************************************************************/
void bch_hprint(const char *name, bch_p op) {
	bch_size i;
	assert(op != 0);
	if (bch_is_negative(op)) {
		bch_p op_ = bch_clone(op);
		bch_negate(op_);
		int32_t exp = bch_hexp(op_);
		printf("%s: -", name);
		for (i = exp; i >= 0; i--) {
			printf("%02X",op_->data[i]);
			if (!i) break;
		}
		bch_free(op_);
	}
	else {
		int32_t exp = bch_hexp(op);
		if (exp == -1) {
			printf("%s: 0", name);
		}
		else {
			printf("%s: ", name);
			for (i = exp; i >= 0; i--) {
				printf("%02X",op->data[i]);
				if (!i) break;
			}
		}
	}
	printf("\n\r");
}

void bch_print(const char *name, bch_p op) {
	bch_size i;
	assert(op != 0);
	if (bch_is_negative(op)) {
		bch_p op_ = bch_clone(op);
		bch_negate(op_);
		int32_t exp = bch_hexp(op_);
		printf("%s-0x", name);
		for (i = exp; i >= 0; i--) {
			printf("%02X",op_->data[i]);
			if (!i) break;
		}
		bch_free(op_);
	}
	else {
		int32_t exp = bch_hexp(op);
		if (exp == -1) {
			printf("%s0x0", name);
		}
		else {
			printf("%s0x", name);
			for (i = exp; i >= 0; i--) {
				printf("%02X",op->data[i]);
				if (!i) break;
			}
		}
	}
	printf("\n\r");
}

bch_p bch_random_gen(bch_p dst, bch_p max, bch_random_p g) {
	uint16_t i;
	uint16_t max_exp = dst->size;
	if (max) max_exp = BCH_MIN(max_exp,bch_hexp(max) + 1);

	for (i = 0; i < max_exp; ++i) {
		if (i == max_exp - 1) {
			if (max) {
				dst->data[i] = (*g->random)(max->data[i]-1);
			} else {
				dst->data[i] = (*g->random)(0xEF);
			}
		}
		else  {
			dst->data[i] = (*g->random)(0xEF);
		}
	}

	return dst;
}

bch_p bch_alloc(bch_size size) {
	bch_p res = 0;
	res = malloc(size * sizeof(bch));
	if (res)
		res->size = size;
	else
		DBG_print("Unable to allocate memory for bit chain.\n\r");
	res->data = (bch_data_p)malloc(size * sizeof(bch_data));
	if (res->data) {
		bch_zero(res);
	}
	else {
		free(res);
		DBG_print("Unable to allocate memory for bit chain data (size %d).\n\r", size);
		res = 0;
	}
	return res;
}

bch_p bch_from_ba(bch_size dst_size, bch_data_p src, bch_size src_size) {
	bch_p res = bch_alloc(dst_size);

	assert(res);
	assert(res->data);

	memcpy(res->data, src, BCH_MIN(dst_size,src_size));

	return res;
}

bch_p bch_copy(bch_p dst, bch_p src) {

	assert(dst);
	assert(src);

	bch_size copy_size = BCH_MIN(dst->size, src->size);

	bch_zero(dst);
	memcpy(dst->data,src->data,copy_size * sizeof(bch_data));

	return dst;
}

bch_p bch_clone(bch_p src) {
	bch_p clone = bch_alloc(src->size);
	if (clone) {
		bch_copy(clone,src);
	} else {
		DBG_print("Unable to clone bit chain.\n\r");
	}
	return clone;
}

void bch_free(bch_p ptr) {
	if (ptr && ptr->data) {
		free(ptr->data);
	}
	if (ptr) {
		free(ptr);
	}
}

bch_p bch_rev(bch_p s) {
	int32_t bexp = bch_hexp(s);
	if (bexp >= 0) {
		bch_size i, ln = s->size;
		bch_data c;
		for (i = 0; i < ln/2; ++i) {
			c = s->data[ln-1-i];
			s->data[ln-1-i] = s->data[i];
			s->data[i] = c;
		}
		bch_byte_shr(s,s->size - bexp - 1);
	}
}

/********************************************************************************************************
 * Code
 ********************************************************************************************************/
bch_p bch_zero(bch_p op) {
	assert(op != 0);
	assert(op->data != 0);

	memset(op->data, 0, op->size * sizeof(bch_data));
	return op;
}

bch_p bch_one(bch_p op) {
	assert(op != 0);
	assert(op->data != 0);

	memset(op->data, 0, op->size * sizeof(bch_data));
	op->data[0] = 1;

	return op;
}

int8_t bch_is_negative(bch_p op) {

	assert(op != 0);

	return op->data[op->size-1] & 0x80 ? 1 : 0;
}

int8_t bch_is_zero(bch_p op) {
	int8_t res = -1;
	bch_size i;

	assert(op != 0);

	for (i = 0; i < op->size; i++) {
		if (op->data[i]) {
			res = 0;
			break;
		}
	}
	return res;
}

int32_t bch_hexp(bch_p op) {

	int32_t res = -1;
	bch_size i;

	assert(op != 0);

	i = op->size - 1;
	do {
		if (op->data[i]) {
			res = i;
			break;
		}
	} while (i-- > 0);

	return res;
}

int32_t bch_bexp(bch_p op) {

	int32_t res = -1;
	bch_size i;

	assert(op != 0);

	i = op->size - 1;
	bch_data v = 0;
	do {
		v = op->data[i];
		if (v) {
			res = i * 8;
			break;
		}
	} while (i-- > 0);
	i = 0;
	if (v) {
		i = 8;
		bch_data mask = 0x80;
		while (! (v & mask) ) {
			i--;
			mask >>= 1;
		}
	}
	return res + i;
}

bch_p bch_add(bch_p dst, bch_p add) {
	bch_size i, loop_cnt;
	bch_data2x v = 0;

	assert(dst != 0);
	assert(add != 0);

	loop_cnt = BCH_MIN(dst->size,add->size);
	for (i = 0; i < loop_cnt; i++) {
		v += dst->data[i] + add->data[i];
		dst->data[i] = v & 0xFF;
		v -= v & 0xFF;
		v >>= 8;
	}
	if (v && dst->size-1 > i) {
		do {
			v += dst->data[i];
			dst->data[i] = v & 0xFF;
			v -= v & 0xFF;
			v >>= 8;
		} while (v && i++ < dst->size);
	}
	return dst;
}

bch_p bch_add_s(bch_p dst, bch_data add) {
	bch_data2x v = add;

	assert(dst != 0);

	int i = 0;
	do {
		if (i < dst->size) {
			v += dst->data[i];
			dst->data[i] = v & 0xFF;
		}
		v -= v & 0xFF;
		v >>= 8;
	} while (v && i++ < dst->size);

	return dst;
}

bch_p bch_sub(bch_p dst, bch_p sub) {

	assert(dst);
	assert(sub);

	bch_p add = bch_negate(bch_clone(sub));

	assert(add);

	bch_add(dst,add);
	bch_free(add);
	return dst;
}

bch_p bch_negate(bch_p op) {
	bch_size i;

	assert(op != 0);

	for (i = 0; i < op->size; ++i) {
		op->data[i] ^= 0xFF;
	}
	bch_add_s(op,1);
	return op;
}

bch_p bch_abs(bch_p op) {
	if (bch_is_negative(op)) {
		bch_negate(op);
	}
	return op;
}

int8_t bch_cmp(bch_p l, bch_p r) {
	int8_t res = 0;
	uint8_t invert = 0;

	assert(l != 0);
	assert(r != 0);

	/* Step 1: Compare signs */
	uint8_t	ls = bch_is_negative(l),
			rs = bch_is_negative(r);
	if (ls != rs) {
		res = ls - rs;
		return res;
	}
	if (ls) {
		invert = ls == rs;
	}

	/* Step 2: Exponents */
	bch_p l_ = bch_abs(bch_clone(l));
	bch_p r_ = bch_abs(bch_clone(r));

	int32_t	le = bch_bexp(l_),
			re = bch_bexp(r_);
	if (le < 0 && re >= 0)
		res = -1;
	if (re < 0 && le >= 0)
		res = 1;
	if (le < 0 && re < 0) {
		res = 0;
		return res;
	}
	if (!res && le != re) {
		res = (le - re) > 0 ? 1 : -1;
	}

	/* Step 3: Digits */
	int32_t hexp = bch_hexp(l_);
	if (!res && hexp >= 0) {
		int32_t i = hexp;
		do {
			if (l_->data[i] != r_->data[i]) {
				res = (l_->data[i] - r_->data[i]) > 0 ? 1 : -1;
				break;
			}
			--i;
		} while (i >= 0);
	}

	bch_free(l_);
	bch_free(r_);

	if (invert && res) {
		res = res == 1 ? -1 : 1;
	}
	return res;
}

bch_p bch_mul_s(bch_p dst, bch_data mul) {
	bch_size i, loop_cnt;
	bch_data2x v = 0;
	bch_p dst_ = dst;

	assert(dst != 0);

	uint8_t sign = bch_is_negative(dst);
	if (sign) {
		dst_ = bch_abs(bch_clone(dst));
	}

	loop_cnt = bch_hexp(dst_) + 1;
	for (i = 0; i < loop_cnt; i++) {
		v += dst_->data[i] * mul;
		dst_->data[i] = v & 0xFF;
		v -= v & 0xFF;
		v >>= 8;
	}
	if (v && dst_->size-1 > i) {
		dst_->data[i] += v & 0xFF;
	}

	if (sign) {
		bch_negate(dst_);
		bch_copy(dst,dst_);
		bch_free(dst_);
	}

	return dst;
}

bch_p bch_byte_shl(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_p tmp = bch_clone(dst);
	bch_zero(dst);
	memcpy(dst->data + shift, tmp->data, (dst->size - shift) * sizeof(bch_data));
	bch_free(tmp);
	return dst;
}

bch_p bch_byte_shr(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_p tmp = bch_clone(dst);
	bch_zero(dst);
	memcpy(dst->data, tmp->data + shift, (dst->size - shift) * sizeof(bch_data));
	bch_free(tmp);
	return dst;
}

bch_p bch_bit_shl(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_size byte_shift = shift / 8;
	bch_size bit_shift = shift % 8;

	bch_data l_mask = 0xFF << (8 - bit_shift);

	if (byte_shift)
		bch_byte_shl(dst,byte_shift);

	bch_size i;
	bch_data v, tmp = 0;
	for (i = 0; i < dst->size; ++i) {
		v = dst->data[i];
		v = (v << bit_shift) | (tmp >> (8 - bit_shift));
		tmp = dst->data[i] & l_mask;
		dst->data[i] = v;
	}

	return dst;
}

bch_p bch_bit_shr(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_size byte_shift = shift / 8;
	bch_size bit_shift = shift % 8;

	bch_data r_mask = 0xFF >> bit_shift;

	if (byte_shift)
		bch_byte_shr(dst,byte_shift);

	bch_size i;
	bch_data v, tmp = 0;
	for (i = dst->size - 1; i >= 0; --i) {
		v = dst->data[i];
		v = (v >> bit_shift) | (tmp << (8 - bit_shift));
		tmp = dst->data[i] & r_mask;
		dst->data[i] = v;
		if (i == 0) break;
	}

	return dst;
}

bch_p bch_mul(bch_p dst, bch_p mul) {

	assert(dst != 0);

	bch_p dst_ = 0;
	bch_p dstmul_ = 0;
	bch_p mul_ = mul;

	int8_t	dsts = bch_is_negative(dst),
			muls = bch_is_negative(mul_);

	int8_t negate_result = dsts != muls;
	if (dsts) {
		dst_ = bch_abs(bch_clone(dst));
	}
	else {
		dst_ = bch_clone(dst);
	}
	if (muls) {
		mul_ = bch_abs(bch_clone(mul));
	}

	dstmul_ = bch_clone(dst_);

	bch_size i,j;
	bch_data2x v,u;

	int32_t mul_hexp = bch_hexp(mul_);
	int32_t dst_hexp = bch_hexp(dstmul_);
	bch_zero(dst_);

	for (i = 0; i < mul_hexp + 1; ++i) {
		v = 0; u = 0;
		for (j = 0; j < dst_hexp + 1; ++j) {
			v = v + u + ( mul_->data[i] * dstmul_->data[j] );
			if (dst_->size > i+j) {
				u = dst_->data[i+j] + (v & 0xFF);
				dst_->data[i+j] = u & 0xFF;
			}
			v -= v & 0xFF; u -= u & 0xFF;
			v >>= 8; u >>= 8;
		}
		if ((v+u) && dst_->size > i+j) {
			do {
				v = v + u;
				if (dst_->size > i+j) {
					u = dst_->data[i+j] + (v & 0xFF);
					dst_->data[i+j] = u & 0xFF;
				}
				v -= v & 0xFF; u -= u & 0xFF;
				v >>= 8; u >>= 8;
			} while (v+u);
		}
	}

	bch_free(dstmul_);
	bch_copy(dst, dst_);
	bch_free(dst_);

	if (muls) {
		bch_free(mul_);
	}
	if (negate_result) {
		bch_negate(dst);
	}

	return dst;
}

void bch_div_mod(bch_p r, bch_p m, bch_p divided, bch_p divider) {

	assert(divided != 0);
	assert(divider != 0);

	bch_p divided_	= divided;
	bch_p divider_	= divider;
	bch_p m_ 		= m;

	int8_t	divideds = bch_is_negative(divided_),
			dividers = bch_is_negative(divider_);

	int8_t negate_result = divideds != dividers;
	if (divideds) {
		divided_ = bch_abs(bch_clone(divided));
	}
	if (dividers) {
		divider_ = bch_abs(bch_clone(divider));
	}

	/* divider > divided */
	if ( bch_cmp(divider_,divided_) > 0 ) {
		if (r) bch_zero(r);
		if (m) bch_copy(m,divided_);
		goto exit_func;
	}

	if (!m_) {
		m_ = bch_clone(divided_);
	}
	else {
		bch_zero(m_);
		bch_copy(m_,divided_);
	}
	if (r) {
		bch_zero(r);
	}

	assert(divided_ != 0);
	assert(divider_ != 0);
	assert(m_ != 0);

	int32_t divided_exp = bch_bexp(divided_);
	/* Zero divided */
	if (divided_exp < 0) {
		if (r) bch_zero(r);
		goto exit_func;
	}
	int32_t divider_exp = bch_bexp(divider_);
	/* Zero divider */
	assert(divider_exp >= 0);

	int32_t exp = (divided_exp-divider_exp);

	bch_p sub = bch_alloc(divided_->size);
	bch_p add = bch_alloc(divided_->size);

	while(bch_cmp(m_, divider_) >= 0) {

		if (exp < 0)
			break;

		bch_copy(sub, divider_);

		if (exp) bch_bit_shl(sub, exp);

		if (bch_cmp(sub,m_) <= 0) {
			bch_sub(m_,sub);
			if (r) {
				bch_zero(add);
				bch_add_s(add,1);
				if (exp) bch_bit_shl(add,exp);
				bch_add(r,add);
			}
		}

		--exp;
	}

	bch_free(sub);
	bch_free(add);

exit_func:
	if (divideds) {
		bch_free(divided_);
	}
	if (dividers) {
		bch_free(divider_);
	}
	if (!m) {
		bch_free(m_);
	}
	if (r && negate_result) {
		bch_negate(r);
	}
}

bch_p bch_div(bch_p dst, bch_p div) {

	assert(dst);
	assert(div);

	bch_p divided = bch_clone(dst);
	bch_div_mod(dst, 0, divided, div);

	bch_free(divided);

	return dst;
}

bch_p bch_mod(bch_p dst, bch_p div) {

	assert(dst);
	assert(div);

	bch_p divided = bch_clone(dst);
	bch_div_mod(0, dst, divided, div);

	bch_free(divided);

	return dst;
}

bch_p bch_gcd(bch_p dst, bch_p a, bch_p b) {

	bch_p mod	= bch_alloc(dst->size);
	bch_p a_	= bch_clone(a);
	bch_p b_	= bch_clone(b);

	while (!bch_is_zero(b_)) {
		bch_div_mod(0,mod,a_,b_);

		bch_copy(a_,b_);
		bch_copy(b_,mod);
	}

	bch_copy(dst,a_);

	bch_free(mod);
	bch_free(a_);
	bch_free(b_);

	return dst;
}

bch_p bch_gcdex(bch_p dst, bch_p a, bch_p b, bch_p x, bch_p y) {

	if (bch_is_zero(b)) {
		bch_copy(dst,a);
		bch_zero(y);
		bch_zero(x);
		bch_add_s(x,1);
		return dst;
	}

	bch_p	q	= bch_alloc(dst->size),
			r	= bch_alloc(dst->size),
			x1 	= bch_alloc(x->size),
			x2 	= bch_alloc(x->size),
			y1 	= bch_alloc(y->size),
			y2 	= bch_alloc(y->size),
			t 	= bch_alloc(y->size),
			a_	= bch_clone(a),
			b_	= bch_clone(b);


	bch_add_s(x2,1);
	bch_add_s(y1,1);

	while (!bch_is_zero(b_)) {
		bch_div_mod(q,r,a_,b_);

		bch_copy(t,x1);
		bch_mul(t,q);
		bch_copy(x,x2);
		bch_sub(x,t);

		bch_copy(t,y1);
		bch_mul(t,q);
		bch_copy(y,y2);
		bch_sub(y,t);

		bch_copy(a_,b_);
		bch_copy(b_,r);
		bch_copy(b_,r);

		bch_copy(x2,x1);
		bch_copy(x1,x);
		bch_copy(y2,y1);
		bch_copy(y1,y);
	}

	bch_copy(dst,a_);
	bch_copy(x,x2);
	bch_copy(y,y2);

	bch_free(q);
	bch_free(r);
	bch_free(x1);
	bch_free(x2);
	bch_free(y1);
	bch_free(y2);
	bch_free(t);
	bch_free(a_);
	bch_free(b_);

	return dst;
}

bch_p bch_inverse(bch_p dst, bch_p a, bch_p n) {
	bch_p result = 0;
	bch_p tmp = bch_alloc(dst->size);
	bch_p x = bch_alloc(dst->size);
	bch_p y = bch_alloc(dst->size);

	bch_add_s(tmp,1);

	bch_gcdex(dst, a, n, x, y);
	if (bch_cmp(dst,tmp) == 0) {
		if (bch_is_negative(x)) {
			bch_add(x,n);
		}
		bch_copy(dst,x);
		result = dst;
	}

	bch_free(x);
	bch_free(y);
	bch_free(tmp);

	return result;
}

bch_p bch_pow(bch_p x,bch_p y) {

	bch_p x_ = bch_clone(x);
	bch_p y_ = bch_clone(y);
	bch_p s = bch_alloc(x->size);
	bch_add_s(s,1);

	while(!bch_is_zero(y_)) {
		if (y_->data[0] & 1) {
			bch_mul(s,x_);
		}
		bch_bit_shr(y_,1);
		bch_mul(x_,x_);
	}

	bch_copy(x,s);

	bch_free(x_);
	bch_free(y_);
	bch_free(s);

	return x;
}

bch_p bch_powmod(bch_p x,bch_p y,bch_p n) {

	bch_p x_ = bch_clone(x);
	bch_p y_ = bch_clone(y);
	bch_p s = bch_alloc(x->size);
	bch_add_s(s,1);

	while(!bch_is_zero(y_)) {
		if (y_->data[0] & 1) {
			bch_mul(s,x_);
			bch_mod(s,n);
		}
		bch_bit_shr(y_,1);
		bch_mul(x_,x_);
		bch_mod(x_,n);
	}

	bch_copy(x,s);

	bch_free(x_);
	bch_free(y_);
	bch_free(s);

	return x;
}


/********************************************************************************************************
 * Test code
 ********************************************************************************************************/
