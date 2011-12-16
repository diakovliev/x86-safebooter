#include "dsa.h"

#ifdef __HOST_COMPILE__
void dsa_sign(bch_p H, bch_p r, bch_p s, bch_random_p random) {

	/* Alloc numbers */
	bch_p k =		bch_alloc(DSA_SIZE);
	bch_p k_inv =	bch_alloc(DSA_SIZE);

	bch_p G =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_G, dsa_G_size));
	bch_p P =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_P, dsa_P_size));
	bch_p Q =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_Q, dsa_Q_size));
	bch_p priv =	bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_priv, dsa_priv_size));

	/* Generate random k E [ 0.. G )*/
	bch_p inv_res = 0;
	do {
		bch_random_gen(k,Q,random);
		inv_res = (bch_p)bch_inverse_bin(k_inv,k,Q);
/*		inv_res = (bch_p)bch_inverse(k_inv,k,Q);*/
	} while ( !inv_res );

#ifdef __DEBUG__
	bch_print("k = ",k);
#endif/*__DEBUG__*/

    /* (G ^ k mod P) mod Q */
    bch_copy(r, G);
    bch_powmod(r, k, P);
    bch_mod(r,Q);

	/* x * r mod Q */
	bch_copy(s,priv);
	bch_mulmod(s,r,Q);

	/* H(m) + x*r mod Q */
	bch_add(s,H);
	bch_mod(s,Q);

	/* (k^-1 * (H(m) + x*r)) mod Q */
	bch_mulmod(s,k_inv,Q);

	bch_free(k, k_inv, G, P, Q, priv);
}
#endif/*__HOST_COMPILE__*/

int8_t dsa_check(bch_p H, bch_p r, bch_p s) {
	int8_t res = -1;

	bch_p G =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_G, dsa_G_size));
	bch_p P =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_P, dsa_P_size));
	bch_p Q =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_Q, dsa_Q_size));
	bch_p pub =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_pub, dsa_pub_size));

    bch_p w =		bch_alloc(DSA_SIZE);
    bch_p u1 =		bch_alloc(DSA_SIZE);
    bch_p u2 =		bch_alloc(DSA_SIZE);
    bch_p v =		bch_alloc(DSA_SIZE);
    bch_p t0 =		bch_alloc(DSA_SIZE);
    bch_p t1 =		bch_alloc(DSA_SIZE);

	/* w = s^-1 mod Q*/
	bch_p inv_res = (bch_p)bch_inverse_bin(w,s,Q);
/*	bch_p inv_res = (bch_p)bch_inverse(w,s,Q);*/
	if (inv_res) {

		/* u1 = (H(m) * w) mod Q*/
		bch_copy(u1,w);
		bch_mulmod(u1,H,Q);

		/* u2 = r * w mod Q */
		bch_copy(u2,w);
		bch_mulmod(u2,r,Q);

		/* G ^ u1 mod p */
		bch_copy(t0,G);
		bch_powmod(t0,u1,P);
		/* pub ^ u2 mod p */
		bch_copy(t1,pub);
		bch_powmod(t1,u2,P);

		bch_copy(v,t0);
		bch_mulmod(v,t1,P);
		bch_mod(v,Q);

#ifdef __DEBUG__
		bch_hprint("v", v);
#endif/*__DEBUG__*/

		res = bch_cmp(r,v);
	}

    bch_free(w, u1, u2, v, t0, t1, G, P, Q, pub);

	return res;
}
