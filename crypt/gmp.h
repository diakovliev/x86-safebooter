#ifndef GMP_HEADER
#define GMP_HEADER

#include <stdint.h>

#ifndef GMP_FUNC
#define GMP_FUNC extern
#endif

typedef struct gmp_number_s {
	uint16_t size;
	int16_t *data;
} gmp_number_t, *gmp_number_p;

GMP_FUNC gmp_number_p gmp_tools_clone_number(gmp_number_p src);
GMP_FUNC gmp_number_p gmp_tools_alloc_number(uint16_t size);
GMP_FUNC void gmp_tools_free_number(gmp_number_p ptr);
GMP_FUNC void gmp_tools_dump_number(const char *fmt, gmp_number_p op, uint16_t base);
GMP_FUNC void gmp_tools_copy_number(gmp_number_p dst, gmp_number_p src);

GMP_FUNC uint8_t gmp_tools_is_negate(gmp_number_p op, uint16_t base);
GMP_FUNC int32_t gmp_tools_last_significant_digit(gmp_number_p op);
GMP_FUNC void gmp_tools_create_exp(gmp_number_p res, uint16_t exp, int16_t value);
GMP_FUNC void gmp_tools_shl(gmp_number_p res, gmp_number_p op, uint16_t exp);
GMP_FUNC void gmp_tools_create_zero(gmp_number_p op);
GMP_FUNC uint64_t gmp_tools_get_small(gmp_number_p op, uint16_t base);
GMP_FUNC void gmp_tools_reverse(gmp_number_p op, uint16_t size);
GMP_FUNC int16_t gmp_tools_is_zero(gmp_number_p op);
GMP_FUNC uint32_t gmp_tools_random(uint32_t N);

GMP_FUNC int8_t gmp_cmp(gmp_number_p op0_, gmp_number_p op1_, uint16_t base);
GMP_FUNC void gmp_add(gmp_number_p res, gmp_number_p left, gmp_number_p right, uint16_t base);
GMP_FUNC void gmp_add_s(gmp_number_p res, gmp_number_p left, int16_t right, uint16_t base);
GMP_FUNC void gmp_get_negate(gmp_number_p res, gmp_number_p op, uint16_t base);
GMP_FUNC void gmp_get_abs(gmp_number_p res, gmp_number_p op, uint16_t base);
GMP_FUNC void gmp_sub(gmp_number_p res, gmp_number_p left_, gmp_number_p right_, uint16_t base);
GMP_FUNC void gmp_sub_s(gmp_number_p res, gmp_number_p left, int16_t right, uint16_t base);
GMP_FUNC void gmp_mul_s(gmp_number_p res, gmp_number_p left_, int16_t right, uint16_t base);
GMP_FUNC void gmp_mul(gmp_number_p res, gmp_number_p left_, gmp_number_p right_, uint16_t base);
GMP_FUNC void gmp_div_s_f(gmp_number_p res, uint16_t *mod, gmp_number_p divided_, int16_t divider, uint16_t base);
GMP_FUNC void gmp_div_s(gmp_number_p res, gmp_number_p divided, int16_t divider, uint16_t base);
GMP_FUNC uint16_t gmp_mod_s(gmp_number_p divided, int16_t divider, uint16_t base);
GMP_FUNC void gmp_div_f(gmp_number_p res, gmp_number_p mod, gmp_number_p divided_, gmp_number_p divider_, uint16_t base);
GMP_FUNC void gmp_div(gmp_number_p res, gmp_number_p divided, gmp_number_p divider, uint16_t base);
GMP_FUNC void gmp_mod(gmp_number_p res, gmp_number_p divided, gmp_number_p divider, uint16_t base);
GMP_FUNC void gmp_pow_s(gmp_number_p res, gmp_number_p op, uint16_t power, uint16_t base);
GMP_FUNC void gmp_pow(gmp_number_p res, gmp_number_p a, gmp_number_p power, uint16_t base);
GMP_FUNC void gmp_powmod(gmp_number_p res, gmp_number_p a_, gmp_number_p power, gmp_number_p mod, uint16_t base);
GMP_FUNC void gmp_gcd(gmp_number_p res, gmp_number_p a_, gmp_number_p b_, uint16_t base);
GMP_FUNC void gmp_gcdex(gmp_number_p d, gmp_number_p a_, gmp_number_p b_, gmp_number_p x, gmp_number_p y, uint16_t base);
GMP_FUNC uint8_t gmp_inverse(gmp_number_p res, gmp_number_p a, gmp_number_p n, uint16_t base);

#if 0
GMP_FUNC void test_gmp(void);
#endif


#endif /* GMP_HEADER */
