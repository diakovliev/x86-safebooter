/*
 * bch.c
 *
 *  Created on: Aug 8, 2011
 *      Author: D.Iakovliev
 */

#include "bch.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "../core/debug.h" /* assert */

/********************************************************************************************************
 * Memory trace
 ********************************************************************************************************/
#ifdef __DEBUG__
static unsigned long allocated = 0;
static unsigned long released = 0;
static unsigned long max_usage = 0;
#endif

static inline void *bch__alloc(size_t size) {
#ifdef __DEBUG__
	++allocated;
	max_usage = BCH_MAX(max_usage, allocated-released);
#endif
	return (void*)malloc(size);
}

static inline void bch__free(void *ptr) {
#ifdef __DEBUG__
	++released;
#endif
	free(ptr);
}

#ifdef __DEBUG__

#include "dsa_base.h"

void bch__memory_usage() {
	printf("DEBUG: BCH memory usage %ld (%ld Kb).\n\r", max_usage, (max_usage * DSA_SIZE) / 1024);
}

#endif

#define BCH_FFS(x) \
	{\
		x =  ((x & 0xff000000) >> 24)\
		   | ((x & 0x00ff0000) >> 8)\
		   | ((x & 0x0000ff00) << 8)\
		   | ((x & 0x000000ff) << 24);\
		x =  ((x & 0xf0f0f0f0) >> 4)\
		   | ((x & 0x0f0f0f0f) << 4);\
		x =  ((x & 0x88888888) >> 3)\
		   | ((x & 0x44444444) >> 1)\
		   | ((x & 0x22222222) << 1)\
		   | ((x & 0x11111111) << 3);\
		i = x;\
	}

#if 0	
#define uint8_data(val,index) *((uint8_t*)(val+(index)))
#define uint16_data(val,index) *((uint16_t*)(val+(index)))
#define uint32_data(val,index) *((uint32_t*)(val+(index)))
#endif
//#if 0	
#define uint8_data(val,index) *(uint8_t*)&(val)[index]
#define uint16_data(val,index) *(uint16_t*)&(val)[index]
#define uint32_data(val,index) *(uint32_t*)&(val)[index]
//#endif

#define xdata(x) x->data

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
	res = (bch_p)bch__alloc(sizeof(bch));
	if (res) {
		res->size = size;
		res->data = (bch_data_p)bch__alloc(((size/4)+(size%4?1:0))*4*sizeof(bch_data));
		bch_zero(res);
	}
	else {
		free(res);
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

#if 0
bch_p bch_from_string(bch_size dst_size, const char *presentation, unsigned char base) 
{
	bch_p res = bch_alloc(dst_size);

	assert(res);
	assert(res->data);

	size_t len = strlen(presentation);
	const char *ptr = presentation + len - 1;
	bch_data byte = 0;
	char buffer[3];
	buffer[2] = 0;
	bch_data_p dst_ptr = res->data;
	uint8_t is_set = 0;
	while (ptr > presentation) {
		buffer[0] = *(ptr - 1);
		buffer[1] = *(ptr);
		//printf("%s = ", buffer);
		byte = BCH_ABS(strtol(buffer, 0, base));
		//printf("0x%02X\n", byte);
		if (res->data + res->size > dst_ptr) {
			*dst_ptr = byte;
			++dst_ptr;
		}
		ptr -= 2;
		is_set = 1;
	}
	if (is_set && buffer[0] == '-') {
		bch_negate(res);
	}

	return res;
}
#endif

bch_p bch_copy(bch_p dst, bch_p src) {

	assert(dst);
	assert(src);

	bch_size copy_size = BCH_MIN(dst->size, src->size);

	if (dst->size < src->size) {
		bch_zero(dst);
	}

	memcpy(dst->data,src->data,copy_size * sizeof(bch_data));
	//dst->size = copy_size;

	return dst;
}

bch_p bch_clone(bch_p src) {
	bch_p clone = bch_alloc(src->size);
	if (clone) {
		bch_copy(clone,src);
	}
	return clone;
}

void bch_free_raw(bch_p ptr) {
	if (ptr && ptr->data) {
		bch__free(ptr->data);
	}
	if (ptr) {
		bch__free(ptr);
	}
}

void bch_va_free(bch_p ptr,...) {
	va_list ap;
	bch_p p = ptr;
	va_start(ap,ptr);
	do {
		bch_free_raw(p);
	} while ((p = va_arg(ap,bch_p)));
	va_end(ap);
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
		bch_byte_shr(s,s->size-bexp-1);
	}
	return s;
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
	
bch_size bch_ffs(bch_data v) {

	bch_size i;

	BCH_FFS(v);

	return i;
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

	BCH_FFS(v);

	return res + i;
}

static inline
void
bch_add_arr(
	bch_data_p dst, bch_size dst_size,
	bch_data_p add, bch_size add_size,
	char sub
	)
{
	/*
	bch_size i, loop_cnt;
	bch_data2x v = 0;

	loop_cnt = BCH_MIN(dst->size,add->size);
	for (i = 0; i < loop_cnt; i++) {
		v += dst->data[i] + add->data[i];
		dst->data[i] = v & 0xFF;
		v -= dst->data[i];
		v >>= 8;
	}
	if (v && dst->size-1 > i) {
		do {
			v += dst->data[i];
			dst->data[i] = v & 0xFF;
			v -= dst->data[i];
			v >>= 8;
		} while (v && i++ < dst->size);
	}
	return dst;
	*/

	bch_size i, loop_cnt;
	uint32_t v = sub ? 1 : 0;

	loop_cnt = ((BCH_MIN(dst_size,add_size)/2)+1)*2;
	for (i = 0; i < loop_cnt && i < dst_size; i += 2) {
		//uint16_t a = sub ? (*(uint16_t*)(add+i)) ^ 0xFFFF : *(uint16_t*)(add+i);
		//v += *(uint16_t*)(dst+i) + a;
		//*(uint16_t*)(dst+i) = v & 0xFFFF;
		v += uint16_data(dst,i) + (sub ? uint16_data(add,i) ^ 0xFFFF : uint16_data(add,i));
		uint16_data(dst,i) = v & 0xFFFF;
		v >>= 16;
	}
	while (v && dst_size-1 > i) {
		if (i < dst_size) {
			//v += *(uint16_t*)(dst+i);
			//*(uint16_t*)(dst+i) = v & 0xFFFF;
			v += uint16_data(dst,i);
			uint16_data(dst,i) = v & 0xFFFF;
			v >>= 16;
		}
		else break;
		i+=2;
	}
}

bch_p bch_add(bch_p dst, bch_p add) {

	assert(dst);
	assert(add);

	bch_add_arr(dst->data,dst->size, add->data, add->size, 0);

	return dst;
}

bch_p bch_sub(bch_p dst, bch_p sub) {

	assert(dst);
	assert(sub);

	bch_add_arr(dst->data,dst->size, sub->data, sub->size, 1);

	return dst;
}

bch_p bch_add_s(bch_p dst, bch_data add) {
	uint32_t v = add;

	assert(dst != 0);

	bch_size i = 0;
	while (v && i < dst->size) {
		//v += *(uint16_t*)(dst->data+i);
		//*(uint16_t*)(dst->data+i) = v & 0xFFFF;
		v += uint16_data(dst->data,i);
		uint16_data(dst->data,i) = v & 0xFFFF;
		v >>= 16;
		i += 2;
	}

	return dst;
}

bch_p bch_negate(bch_p op) {
	bch_size i;

	assert(op != 0);

	for (i = 0; i < op->size; i += 4) {
		//*(uint32_t*)(op->data + i) ^= 0xFFFFFFFF;
		uint32_data(op->data,i) ^= 0xFFFFFFFF;
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

	/* Step 2: Compare data */
	bch_p l_ = ls ? bch_abs(bch_clone(l)) : l;
	bch_p r_ = rs ? bch_abs(bch_clone(r)) : r;

	int32_t i;
	bch_data l_data, r_data;
	for (i = BCH_MAX(l_->size,r_->size) - 1; i >= 0; --i) {
		l_data = (i < l_->size) ? l_->data[i] : 0;
		r_data = (i < r_->size) ? r_->data[i] : 0;
		if (l_data != r_data) {
			res = ((l_data - r_data) > 0) ? 1 : -1;
/*
		res = l_data - r_data;
		if (res) {
#if 0
			res = res / BCH_ABS(res);
#endif
			res = res > 0 ? 1 : -1;
*/
			break;
		}
	}

	if (ls) bch_free(l_);
	if (rs) bch_free(r_);

	if (invert && res) {
		res = -res;
	}
	return res;
}

bch_p bch_mul_s(bch_p dst, bch_data mul) {

	bch_size i;
	bch_p dst_ = dst;

	assert(dst != 0);

	uint8_t sign = bch_is_negative(dst);
	if (sign) {
		dst_ = bch_abs(bch_clone(dst));
	}

#if 0
	bch_size loop_cnt;
	bch_data2x v = 0;
	loop_cnt = bch_hexp(dst_) + 1;
	for (i = 0; i < loop_cnt || v; ++i) {
		if (dst_->size > i) {
			v += dst_->data[i] * mul;
			dst_->data[i] = v & 0xFF;
		} else {
			break;
		}
		v >>= 8;
	}
#endif

	uint32_t v = 0;
	for (i = 0; i < dst->size || v; i += 2) {
		if (dst_->size > i) {
			v += uint16_data(dst_->data,i) * mul;
			uint16_data(dst_->data,i) = v & 0xFFFF;
		} else {
			break;
		}
		v >>= 16;
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

	bch_size sz = (dst->size-shift);
	bch_data_p buffer = bch__alloc(sz);

	if (dst->size - shift > 0) {
		memcpy(buffer, dst->data, sz);
		memset(dst->data, 0, dst->size - sz);
		memcpy(dst->data + shift, buffer, sz);
	}

	bch__free(buffer);

	return dst;
}

bch_p bch_byte_shr(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_size sz = (dst->size-shift);
	bch_data_p buffer = bch__alloc(sz);

	if (dst->size - shift > 0) {
		memcpy(buffer, dst->data + shift, sz);
		memset(dst->data + sz, 0, dst->size - sz);
		memcpy(dst->data, buffer, sz);
	}

	bch__free(buffer);

	return dst;
}

bch_p bch_bit_shl(bch_p dst, bch_size shift) {

	assert(dst != 0);

	bch_size byte_shift = shift / 8;
	if (byte_shift)
		bch_byte_shl(dst,byte_shift);

	bch_size bit_shift = shift % 8;
	if (!bit_shift)
		return dst;

//	bch_data l_mask = 0xFF << (8 - bit_shift);
//	bch_size i;
//	bch_size size = dst->size;
//	bch_data v, tmp = 0;
//
//	for (i = 0; i < size; ++i) {
//		v = dst->data[i];
//		v = (v << bit_shift) | (tmp >> (8 - bit_shift));
//		tmp = dst->data[i] & l_mask;
//		dst->data[i] = v;
//	}

	uint32_t l_mask = 0xFFFFFFFF << (32-bit_shift);
	bch_size i;
	uint32_t v, tmp = 0;

	for (i = 0; i < dst->size; i += 4) {
		//v = *((uint32_t*)(dst->data + i));
		v = uint32_data(dst->data,i);
		v = (v << bit_shift) | (tmp >> (32-bit_shift));
		//tmp = *((uint32_t*)(dst->data + i)) & l_mask;
		//*((uint32_t*)(dst->data + i)) = v;
		tmp = uint32_data(dst->data,i) & l_mask;		
		uint32_data(dst->data,i) = v;
	}

	return dst;
}

bch_p bch_bit_shr(bch_p dst, bch_size shift) {

	assert(dst != 0);

	if (shift == 0)
		return dst;

	bch_size byte_shift = shift / 8;
	if (byte_shift)
		bch_byte_shr(dst,byte_shift);

	bch_size bit_shift = shift % 8;
	if (!bit_shift)
		return dst;

//	bch_data r_mask = 0xFF >> bit_shift;
//	bch_size i;
//	bch_data v, tmp = 0;
//
//	for (i = dst->size - 1; i >= 0; --i) {
//		v = dst->data[i];
//		v = (v >>  bit_shift) | (tmp << (8-bit_shift));
//		tmp = dst->data[i] & r_mask;
//		dst->data[i] = v;
//		if (i == 0) break;
//	}

	uint32_t r_mask = 0xFFFFFFFF >> bit_shift;
	bch_size i;
	uint32_t v, tmp = 0;

	for (i = dst->size - 4; i >= 0; i -= 4) {
		//if (*((uint32_t*)(dst->data + i)) || tmp) {
		if (uint32_data(dst->data,i) || tmp) {
			//v = *((uint32_t*)(dst->data + i));
			v = uint32_data(dst->data,i);
			v = (v >> bit_shift) | (tmp << (32-bit_shift));
			//tmp = *((uint32_t*)(dst->data + i)) & r_mask;
			//*((uint32_t*)(dst->data + i)) = v;
			tmp = uint32_data(dst->data,i) & r_mask;
			uint32_data(dst->data,i) = v;
		}
		if (i == 0) break;
	}

	return dst;
}

bch_p bch_keep_sign_shift_right(bch_p dst) {

	const uint8_t shift = 1;

	/*
	const bch_size bit_shift = shift % 8;
	const bch_data r_mask = 0xFF >> bit_shift;

	assert(dst != 0);

	bch_size i;
	bch_size size = dst->size;
	bch_data v, tmp = 0;

	uint8_t is_negative = bch_is_negative(dst) ? 0x80 : 0x00;

	for (i = dst->size - 1; i >= 0; --i) {
		v = dst->data[i];
		v = (v >> bit_shift) | (tmp << 7);
		tmp = dst->data[i] & r_mask;
		if (i == dst->size - 1)
			v |= is_negative;
		dst->data[i] = v;
		if (i == 0) break;
	}
	*/

	const bch_size bit_shift = shift % 8;

	uint32_t r_mask = 0xFFFFFFFF >> bit_shift;
	bch_size i;
	uint32_t v, tmp = 0;
	uint32_t is_negative = bch_is_negative(dst) ? 0x80000000 : 0x0;

	for (i = dst->size - 4; i >= 0; i -= 4) {
		//v = *((uint32_t*)(dst->data + i));
		v = uint32_data(dst->data,i);
		v = (v >> bit_shift) | (tmp << (32-bit_shift));
		//tmp = *((uint32_t*)(dst->data + i)) & r_mask;
		tmp = uint32_data(dst->data,i) & r_mask;
		if (i == dst->size - 4)
			v |= is_negative;
		//*((uint32_t*)(dst->data + i)) = v;
		uint32_data(dst->data,i) = v;
		if (i == 0) break;
	}

	return dst;
}


void
bch_mul_arr_base(
	bch_data_p dst, bch_size dst_size,
	bch_data_p op0, bch_size op0_size,
	bch_data_p op1, bch_size op1_size
			)
{
	bch_size i,j;
	uint32_t v,u;

	/*
	bch_data2x v,u;
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
	*/

	for (i = 0; i < op0_size; i += 2) {
		v = 0; u = 0;
		for (j = 0; j < op1_size; j += 2) {
			v = v + u + ( uint16_data(op0,i) * uint16_data(op1,j) );
			if (dst_size > i+j) {
				u = uint16_data(dst,i+j) + (v & 0xFFFF);
				uint16_data(dst,i+j) = u & 0xFFFF;
			}
			v >>= 16; u >>= 16;
		}
		while ((v+u) && dst_size > i+j) {
			v = v + u;
			if (dst_size > i+j) {
				u = uint16_data(dst,i+j) + (v & 0xFFFF);
				uint16_data(dst,i+j) = u & 0xFFFF;
			}
			v >>= 16; u >>= 16;
		}
	}
}

void
bch_sqr_arr_base(
	bch_data_p dst, bch_size dst_size,
	bch_data_p op, bch_size op_size
			)
{
	bch_size i = 0,j = 0;
	uint32_t t = 0,c = 0;

	for (i = 0; i < op_size + 1; ++i) {
		if (2*i < dst_size) {
			t = uint8_data(dst,2*i) + uint8_data(op,i) * uint8_data(op,i);
			uint8_data(dst,2*i) = t & 0xFF;
		}
		c = t >> 8;
		for (j = i + 1; j < op_size + 1; ++j) {

			if (i+j < dst_size) {
				t = uint8_data(dst,i+j) + (2 * uint8_data(op,i) * uint8_data(op,j)) + c;
				uint8_data(dst,i+j) = t & 0xFF;
			}
			c = t >> 8;
		}
		if (i+op_size+1 < dst_size) {
			uint8_data(dst,i+op_size+1) = c;
		}
	}
}

bch_p bch_mul_base(bch_p dst, bch_p mul) {

	assert(dst != 0);
	assert(mul != 0);

	bch_p dst_ = bch_clone(dst);
	bch_p mul_ = mul;

	bch_p dstmul_ = bch_clone(dst);
	bch_zero(dst_);

	bch_mul_arr_base(dst_->data, dst_->size,
			dstmul_->data, dstmul_->size,
			mul_->data, mul_->size);

	bch_copy(dst, dst_);

	bch_free(dstmul_,dst_);

	return dst;
}

bch_p bch_sqr_base(bch_p dst) {

	assert(dst != 0);

	bch_p dst_ = bch_abs(bch_clone(dst));
	int32_t dst_hexp = bch_hexp(dst_);
	bch_p dstmul_ = bch_clone(dst);
	bch_zero(dst_);

	bch_sqr_arr_base(dst_->data, dst_->size,
			dstmul_->data, dst_hexp + 1);

	bch_copy(dst, dst_);

	bch_free(dstmul_,dst_);

	return dst;
}

bch_p bch_mul(bch_p dst, bch_p mul) {

	assert(dst != 0);

	bch_p dst_ = 0;
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

	bch_mul_base(dst_,mul_);

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

bch_p bch_sqr(bch_p dst) {

	assert(dst != 0);

	bch_sqr_base(dst);

	return dst;
}

/* O(divided_bit_len - divider_bit_len) */
void bch_div_mod_bin_internal(bch_p r, bch_p m_, bch_p divided_, bch_p divider_)
{
	int32_t divided_exp = bch_bexp(divided_);
	/* Zero divided */
	if (divided_exp < 0) {
		if (r) bch_zero(r);
		return;
	}
	int32_t divider_exp = bch_bexp(divider_);
	/* Zero divider */
	if (divider_exp < 0) {
		return;
	}

	int32_t exp = (divided_exp-divider_exp);
	bch_p sub = bch_alloc(divided_->size);

	int32_t m_exp = 0;
	int32_t sub_exp = 0;
	int32_t shift = 0;

	bch_copy(sub, divider_);
	bch_bit_shl(sub, exp);
	sub_exp = bch_bexp(sub);
	m_exp = sub_exp;

	while(exp >= 0) {
		
		if (bch_cmp(m_,sub) >= 0) {
	
			bch_sub(m_,sub);
			if (r) {
				bch_set_bit(r,exp);
			}

			m_exp = bch_bexp(m_);
		}

		shift = (sub_exp > 0 && m_exp > 0 && (BCH_ABS(sub_exp-m_exp) > 0)) 
			? BCH_ABS(sub_exp - m_exp) : 1;
		bch_bit_shr(sub, shift);
		exp 	-= shift;
		sub_exp	-= shift;
	}

	bch_free(sub);
}

/*
	dst = src * mul;
*/
bch_p bch_copy_mul_s(bch_p dst, bch_p src, bch_data mul) {

	bch_size i, loop_cnt;
	bch_data2x v = 0;
	bch_p dst_ = src;

	assert(dst_ != 0);

	bch_zero(dst);

	uint8_t sign = bch_is_negative(dst_);
	if (sign) {
		dst_ = bch_abs(bch_clone(dst_));
	}

	loop_cnt = bch_hexp(dst_) + 1;
	for (i = 0; i < loop_cnt || v; ++i) {
		if (dst_->size > i) {
			v += dst_->data[i] * mul;
			dst->data[i] = v & 0xFF;
		} else {
			break;
		}
		v >>= 8;
	}

	if (sign) {
		bch_negate(dst);
		bch_free(dst_);
	}

	return dst;
}

/*
    dst = dst - (src * mul)
 */
bch_p bch_sub_mul_s(bch_p dst, bch_p src, bch_data mul)
{
	bch_size i;
	uint32_t v = 0,u = 1;
	bch_p sub_ = src;
	uint8_t sign = bch_is_negative(sub_);
	uint16_t xxx = 0xFFFF;

	assert(dst != 0);
	assert(src != 0);
	if (!mul) return dst;

	if (sign) {
		sub_	= bch_clone(bch_abs(sub_));
		u		= 0;
		xxx		= 0;
	}
	for (i = 0; i < dst->size || u; i+=2) {
		if (dst->size <= i) break;
		v += uint16_data(xdata(sub_),i) * mul;
		u += uint16_data(xdata(dst),i);
		u += (v & 0xFFFF) ^ xxx;
		uint16_data(xdata(dst),i) = u & 0xFFFF;
		u >>= 16;
		v >>= 16;
	}
	if (sign) {
		bch_free(sub_);
	}

	return dst;
}

/* O(divided_byte_len - divider_byte_len) */
static void bch_div_mod_internal_smalln(bch_p r, bch_p m_, bch_p divided_, bch_p divider_, int32_t exps[2])
{
	//int32_t divided_exp = bch_hexp(divided_);
	int32_t divided_exp = exps[0];
	/* Zero divided */
	if (divided_exp < 0) {
		if (r) bch_zero(r);
		return;
	}
	//int32_t divider_exp = bch_hexp(divider_);
	int32_t divider_exp = exps[1];
	/* Zero divider */
	if (divider_exp < 0) {
		return;
	}

	int32_t exp 		= (divided_exp-divider_exp);

	int32_t m_exp = divided_exp;
	uint16_t q = 0;
	uint32_t a, b = 0;

	if (divider_exp >= 2) {
		b = (divider_->data[divider_exp] << 16) |
			(divider_->data[divider_exp-1] << 8) |
			(divider_->data[divider_exp-2]);
	} else if (divider_exp == 1) {
		b = (divider_->data[divider_exp] << 16) |
			(divider_->data[divider_exp-1] << 8);
	} else if (divider_exp == 0) {
		b = (divider_->data[divider_exp] << 16);
	}

	bch_byte_shl(divider_,exp++);

	while (exp > 0) {

		if ( bch_cmp(divider_, m_) <= 0 ) {

			a = 0;
			if (m_exp >= 3) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16) |
					(m_->data[m_exp-2] << 8) |
					(m_->data[m_exp-3]);
			}
			else if (m_exp == 2) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16) |
					(m_->data[m_exp-2] << 8);
			}
			else if (m_exp == 1) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16);
			}
			else if (m_exp == 0) {
				a = (m_->data[m_exp] << 24);
			}

			q = a/b;
			if (q & 0xFF00) {
				q >>= 8;
			}
             
			bch_sub_mul_s(m_,divider_,q);

			if (bch_is_negative(m_)) {
				bch_add_arr(m_->data,m_->size, divider_->data, divider_->size, 0);
				q--;
			}

			if (r) {
				bch_set_byte(r,exp-1,q);
			}

			m_exp = bch_hexp(m_);
		}
		
		if (--exp) {
			bch_byte_shr(divider_,1);
		}
	}
}


/* O(divided_byte_len - divider_byte_len) */
static void bch_div_mod_internal_bign(bch_p r, bch_p m_, bch_p divided_, bch_p divider_, int32_t exps[2])
{
	//int32_t divided_exp = bch_hexp(divided_);
	int32_t divided_exp = exps[0];
	/* Zero divided */
	if (divided_exp < 0) {
		if (r) bch_zero(r);
		return;
	}
	//int32_t divider_exp = bch_hexp(divider_);
	int32_t divider_exp = exps[1];
	/* Zero divider */
	if (divider_exp < 0) {
		return;
	}

    bch_p QSP[256];
    bch m_copy;
    memset(QSP, 0, sizeof(QSP));
    QSP[1] = bch_clone(divider_);

	int32_t exp 		= (divided_exp-divider_exp);

	int32_t m_exp = divided_exp;
	uint16_t q = 0;
	uint32_t a, b = 0;

	if (divider_exp >= 2) {
		b = (divider_->data[divider_exp] << 16) |
			(divider_->data[divider_exp-1] << 8) |
			(divider_->data[divider_exp-2]);
	} else if (divider_exp == 1) {
		b = (divider_->data[divider_exp] << 16) |
			(divider_->data[divider_exp-1] << 8);
	} else if (divider_exp == 0) {
		b = (divider_->data[divider_exp] << 16);
	}

	while (exp+1 > 0) {

	    m_copy.data = m_->data;
	    m_copy.size = m_->size;
        m_copy.data += exp;
        m_copy.size -= exp;

		if ( bch_cmp(divider_, &m_copy) <= 0 ) {

			a = 0;
			if (m_exp >= 3) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16) |
					(m_->data[m_exp-2] << 8) |
					(m_->data[m_exp-3]);
			}
			else if (m_exp == 2) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16) |
					(m_->data[m_exp-2] << 8);
			}
			else if (m_exp == 1) {
				a = (m_->data[m_exp] << 24) |
					(m_->data[m_exp-1] << 16);
			}
			else if (m_exp == 0) {
				a = (m_->data[m_exp] << 24);
			}

			q = a/b;
			if (q & 0xFF00) {
				q >>= 8;
			}
             
            if (!QSP[q]) {
                QSP[q] = bch_clone(QSP[1]);
                bch_mul_s(QSP[q],q);
            }
			bch_add_arr(m_->data + exp,m_->size - exp, QSP[q]->data, QSP[q]->size, 1);

			if (bch_is_negative(m_)) {
				bch_add_arr(m_->data + exp,m_->size + exp, divider_->data, divider_->size, 0);
				q--;
			}

			if (r) {
				bch_set_byte(r,exp,q);
			}

			m_exp = bch_hexp(m_);
		}
		
		--exp;
	}

    for (q = 0; q < sizeof(QSP)/sizeof(QSP[0]); ++q)
    {
       if (QSP[q]) bch_free(QSP[q]);
    }
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

	int32_t exps[2];
    exps[0] = bch_hexp(divided_);
	exps[1] = bch_hexp(divider_);

    if (exps[0] <= 256) {
		bch_div_mod_internal_smalln(r,m_,divided_,divider_,exps);
    } else {
		bch_div_mod_internal_bign(r,m_,divided_,divider_,exps);
	}

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

	bch_free(mod,a_,b_);

	return dst;
}

bch_p bch_gcdex_bin(bch_p dst, bch_p x, bch_p y, bch_p a, bch_p b) {

	bch_p g = bch_alloc(dst->size);
	bch_p u = bch_alloc(dst->size);
	bch_p v = bch_alloc(dst->size);
	bch_p A = bch_alloc(dst->size);
	bch_p B = bch_alloc(dst->size);
	bch_p C = bch_alloc(dst->size);
	bch_p D = bch_alloc(dst->size);

	/* 1 */
	bch_set_bit(g,0);

	/* 2 */
	while ( !(x->data[0] & 1) && !(y->data[0] & 1) ) {
		bch_keep_sign_shift_right(x);
		bch_keep_sign_shift_right(y);
		bch_bit_shl(g,1);
	}

	/* 3 */
	bch_copy(u,x);
	bch_copy(v,y);
	bch_set_bit(A,0);
	bch_set_bit(D,0);

	while (!bch_is_zero(u)) {
		/* 4 */
		while ( !(u->data[0] & 1) ) {
			bch_bit_shr(u,1);
			if ( !(A->data[0] & 1) && !(B->data[0] & 1) ) {
				bch_keep_sign_shift_right(A);
				bch_keep_sign_shift_right(B);
			}
			else {
				bch_add(A,y);
				bch_keep_sign_shift_right(A);
				bch_sub(B,x);
				bch_keep_sign_shift_right(B);
			}
		}
		/* 5 */
		while ( !(v->data[0] & 1) ) {
			bch_bit_shr(v,1);
			if ( !(C->data[0] & 1) && !(D->data[0] & 1) ) {
				bch_keep_sign_shift_right(C);
				bch_keep_sign_shift_right(D);
			}
			else {
				bch_add(C,y);
				bch_keep_sign_shift_right(C);
				bch_sub(D,x);
				bch_keep_sign_shift_right(D);
			}
		}
		/* 6 */
		if ( bch_cmp(u,v) >= 0) {
			bch_sub(u,v);
			bch_sub(A,C);
			bch_sub(B,D);
		} else {
			bch_sub(v,u);
			bch_sub(C,A);
			bch_sub(D,B);
		}
	}

	bch_copy(a,C);
	bch_copy(b,D);
	bch_copy(dst,g);
	bch_mul(dst,v);

	bch_free(g,u,v,A,B,C,D);

	return dst;
}

bch_p bch_inverse_bin(bch_p dst, bch_p a, bch_p n) {
	bch_p result = 0;
	bch_p tmp = bch_alloc(dst->size);
	bch_p x = bch_alloc(dst->size);
	bch_p y = bch_alloc(dst->size);

	bch_set_bit(tmp,0);

	bch_gcdex_bin(dst, a, n, x, y);
	if (bch_cmp(dst,tmp) == 0) {
		if (bch_is_negative(x)) {
			bch_add(x,n);
		}
		bch_copy(dst,x);
		result = dst;
	}

	bch_free(x,y,tmp);

	return result;
}

bch_p bch_gcdex(bch_p dst, bch_p a, bch_p b, bch_p x, bch_p y) {

	if (bch_is_zero(b)) {
		bch_copy(dst,a);
		bch_zero(y);
		bch_zero(x);
		bch_set_bit(x,0);
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


	bch_set_bit(x2,0);
	bch_set_bit(y1,0);

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

		bch_copy(x2,x1);
		bch_copy(x1,x);
		bch_copy(y2,y1);
		bch_copy(y1,y);
	}

	bch_copy(dst,a_);
	bch_copy(x,x2);
	bch_copy(y,y2);

	bch_free(q,r,x1,x2,y1,y2,t,a_,b_);

	return dst;
}

bch_p bch_inverse(bch_p dst, bch_p a, bch_p n) {
	bch_p result = 0;
	bch_p tmp = bch_alloc(dst->size);
	bch_p x = bch_alloc(dst->size);
	bch_p y = bch_alloc(dst->size);

	bch_set_bit(tmp,0);

	bch_gcdex(dst, a, n, x, y);
	if (bch_cmp(dst,tmp) == 0) {
		if (bch_is_negative(x)) {
			bch_add(x,n);
		}
		bch_copy(dst,x);
		result = dst;
	}

	bch_free(x,y,tmp);

	return result;
}

bch_p bch_mulmod(bch_p a, bch_p b, bch_p c) {

	bch_p x = bch_alloc(a->size);
	bch_p y = bch_clone(a);

	bch_p b_ = bch_clone(b);

	bch_mod(b_,c);
	bch_mod(y,c);

	int32_t exp = bch_hexp(b_);
	bch_size i = 0;
	if (exp < 0) {
		goto out;
	}

	while (exp + 1) {

		bch_data b_data = b_->data[i++];

#define do_mulmod_step(index) \
		{ \
			if ( b_data & index ) { \
				bch_add(x,y); \
				bch_mod(x,c); \
			} \
			bch_bit_shl(y,1); \
			bch_mod(y,c); \
		}

		do_mulmod_step(0x01);
		do_mulmod_step(0x02);
		do_mulmod_step(0x04);
		do_mulmod_step(0x08);
		do_mulmod_step(0x10);
		do_mulmod_step(0x20);
		do_mulmod_step(0x40);
		do_mulmod_step(0x80);

#undef do_mulmod_step

		exp--;
	}

out:
	bch_copy(a,x);

	bch_free(x,y,b_);

	return a;
}

bch_p bch_pow(bch_p x,bch_p y) {

	bch_p x_ = bch_clone(x);
	bch_p y_ = bch_clone(y);
	bch_p s = bch_alloc(x->size);
	bch_set_bit(s,0);

	int32_t exp = bch_hexp(y_);
	bch_size i = 0;
	if (exp < 0) {
		goto out;
	}

	while(exp + 1) {

		bch_data y_data = y_->data[i++];

#define do_pow_step(index) \
		{ \
			if (y_data & index) { \
				bch_mul(s,x_); \
			} \
			bch_sqr(x_); \
		}

		do_pow_step(0x01);
		do_pow_step(0x02);
		do_pow_step(0x04);
		do_pow_step(0x08);
		do_pow_step(0x10);
		do_pow_step(0x20);
		do_pow_step(0x40);
		do_pow_step(0x80);

#undef do_pow_step

		exp--;
	}

out:
	bch_copy(x,s);

	bch_free(x_,y_,s);

	return x;
}

bch_p bch_powmod(bch_p x,bch_p y,bch_p n) {

	bch_p x_ = bch_clone(x);
	bch_p y_ = bch_clone(y);
	bch_p s = bch_alloc(x->size);
	bch_set_bit(s,0);

	int32_t exp = bch_hexp(y_);
	bch_size i = 0;
	if (exp < 0) {
		goto out;
	}

	while (exp + 1) {

		bch_data y_data = y_->data[i++];

#define do_powmod_step(index) \
		{ \
			if (y_data & index) { \
				bch_mul(s,x_); \
				bch_mod(s,n); \
			} \
			bch_sqr(x_); \
			bch_mod(x_,n); \
		}

		do_powmod_step(0x01);
		do_powmod_step(0x02);
		do_powmod_step(0x04);
		do_powmod_step(0x08);
		do_powmod_step(0x10);
		do_powmod_step(0x20);
		do_powmod_step(0x40);
		do_powmod_step(0x80);

#undef do_powmod_step

		exp--;
	}

out:
	bch_copy(x,s);

	bch_free(x_,y_,s);

	return x;
}

