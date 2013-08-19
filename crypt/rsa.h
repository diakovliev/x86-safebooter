/*
 *	Project: <project>
 *
 *	File: <filename>
 *	Author: <author>
 *	Created: <created date>
 *
 *	Description:
 *
 *
 */

#include "bch.h"

#define RSA_SIZE 512

static inline bch_p rsa_from_ba(bch_data_p src, bch_size src_size) {
	return bch_rev(bch_from_ba(RSA_SIZE,src,src_size));
}

typedef struct rsa_public_key_s {
	bch_p e;
	bch_p n;
} rsa_public_key, *rsa_public_key_p;

static rsa_public_key_p rsa_init_public_key(rsa_public_key_p key) {
	key->e = rsa_from_ba(rsa_e,rsa_e_size);
	key->n = rsa_from_ba(rsa_n,rsa_n_size);
	return key;
}

static void rsa_release_public_key(rsa_public_key_p key) {
	bch_free(key->e,key->n);
}

static inline bch_p rsa_encrypt(/*[in/out]*/ bch_p val, rsa_public_key_p key) {
	return bch_powmod(val,key->e,key->n);
}

typedef struct rsa_private_key_s {
	bch_p p;
	bch_p q;
	bch_p dmp1;
	bch_p dmq1;
	bch_p igmp;
} rsa_private_key, *rsa_private_key_p;

static inline bch_p rsa_decrypt(/*[in/out]*/ bch_p val, rsa_private_key_p key) {

	bch_p m1 = bch_clone(val);
	bch_p m2 = bch_clone(val);
	bch_p h = bch_copy(key->igmp);
	bch_p c = bch_clone(val);
	
	bch_zero(m1);
	bch_zero(m2);

	m1 = bch_powmod(c,key->dmp1,key->p);	
	bch_copy(c,val);
	m2 = bch_powmod(c,key->dmq1,key->q);

	if (bch_cmp(m1,m2) < 0) {
		bch_add(m1,p);
		bch_mod(m1,p);
	}

   	bch_sub(m1,m2);
	bch_mod(m1,p);
	
	bch_mul(h,m1);
	bch_mod(h,p);

	bch_mul(h,key->q);

	bch_copy(val,m2);
	bch_add(val,h);

	bch_free(m1,m2,h,c);

	return val;
}

