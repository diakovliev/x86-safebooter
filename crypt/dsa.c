#include "dsa.h"

#ifdef __HOST_COMPILE__
void dsa_sign(bch_p sha2, bch_p r, bch_p s, bch_random_p random) {

	/* Alloc numbers */
	bch_p k =		bch_alloc(DSA_SIZE);
	bch_p k_inv =	bch_alloc(DSA_SIZE);

	bch_p G =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_G, dsa_G_size));
	bch_p P =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_P, dsa_P_size));
	bch_p Q =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_Q, dsa_Q_size));
//	bch_p pub =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_pub, dsa_pub_size));
	bch_p priv =	bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_priv, dsa_priv_size));
	bch_p sha2_w =	bch_clone(sha2);

	/* Get part of sha2 */
//	int32_t priv_hexp = bch_hexp(priv);
//	int32_t sha2_hexp = bch_hexp(sha2_w);
//	if (sha2_hexp != priv_hexp) {
//		if (sha2_hexp > priv_hexp) {
//			bch_byte_shr(sha2_w,sha2_hexp - priv_hexp);
//		}
//		else {
//			bch_byte_shl(sha2_w,priv_hexp - sha2_hexp);
//		}
//	}

	/* Generate random k E [ 0.. G )*/
	bch_p inv_res = 0;
	do {
		bch_random_gen(k,Q,random);
		//inv_res = (bch_p)bch_inverse(k_inv,k,Q);
		inv_res = (bch_p)bch_inverse_bin(k_inv,k,Q);
	} while ( !inv_res );

    /* (G ^ k mod P) mod Q */
    bch_copy(r, G);
    bch_powmod(r, k, P);
    bch_mod(r,Q);

	/* x * r mod Q */
	bch_copy(s,priv);
	bch_mulmod(s,r,Q);

	/* H(m) + x*r mod Q */
	bch_add(s,sha2_w);
	bch_mod(s,Q);

	/* (k^-1 * (H(m) + x*r)) mod Q */
	bch_mulmod(s,k_inv,Q);

	bch_free(k);
	bch_free(k_inv);

	bch_free(G);
	bch_free(P);
	bch_free(Q);
//	bch_free(pub);
	bch_free(priv);
	bch_free(sha2_w);
}
#endif/*__HOST_COMPILE__*/

int8_t dsa_check(bch_p sha2, bch_p r, bch_p s) {
	int8_t res = -1;

	bch_p G =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_G, dsa_G_size));
	bch_p P =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_P, dsa_P_size));
	bch_p Q =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_Q, dsa_Q_size));
	bch_p pub =		bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_pub, dsa_pub_size));
#ifdef __HOST_COMPILE__
//	bch_p priv =	bch_rev(bch_from_ba(DSA_SIZE, (bch_data_p)dsa_priv, dsa_priv_size));
#endif/*__HOST_COMPILE__*/
	bch_p sha2_w =	bch_clone(sha2);

	/* Get part of sha2 */
#ifdef __HOST_COMPILE__
//	int32_t priv_hexp = bch_hexp(priv);
//	int32_t sha2_hexp = bch_hexp(sha2_w);
//	if (sha2_hexp != priv_hexp) {
//		if (sha2_hexp > priv_hexp) {
//			bch_byte_shr(sha2_w,sha2_hexp - priv_hexp);
//		}
//		else {
//			bch_byte_shl(sha2_w,priv_hexp - sha2_hexp);
//		}
//	}
#endif/*__HOST_COMPILE__*/

    bch_p w =		bch_alloc(DSA_SIZE);
    bch_p u1 =		bch_alloc(DSA_SIZE);
    bch_p u2 =		bch_alloc(DSA_SIZE);
    bch_p v =		bch_alloc(DSA_SIZE);
    bch_p t0 =		bch_alloc(DSA_SIZE);
    bch_p t1 =		bch_alloc(DSA_SIZE);

	/* w = s^-1 mod Q*/
    //bch_p inv_res = (bch_p)bch_inverse(w,s,Q);
	bch_p inv_res = (bch_p)bch_inverse_bin(w,s,Q);
	if (inv_res) {

		/* u1 = (H(m) * w) mod Q*/
		bch_copy(u1,w);
		bch_mul(u1,sha2_w);
		bch_mod(u1, Q);

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

		res = bch_cmp(r,v);
	}

    bch_free(w);
    bch_free(u1);
    bch_free(u2);
    bch_free(v);
    bch_free(t0);
    bch_free(t1);

	bch_free(G);
	bch_free(P);
	bch_free(Q);
	bch_free(pub);
#ifdef __HOST_COMPILE__
//	bch_free(priv);
#endif/*__HOST_COMPILE__*/
	bch_free(sha2_w);

	return res;
}
