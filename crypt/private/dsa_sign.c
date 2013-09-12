#include "dsa_sign.h"

extern unsigned char dsa_pub[];
extern unsigned char dsa_P[];
extern unsigned char dsa_Q[];
extern unsigned char dsa_G[];
extern unsigned int dsa_pub_size;
extern unsigned int dsa_P_size;
extern unsigned int dsa_Q_size;
extern unsigned int dsa_G_size;

extern unsigned char dsa_priv[];
extern unsigned int dsa_priv_size;

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

