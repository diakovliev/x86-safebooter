#include "dsa_check.h"

extern unsigned char dsa_pub[];
extern unsigned char dsa_P[];
extern unsigned char dsa_Q[];
extern unsigned char dsa_G[];
extern unsigned int dsa_pub_size;
extern unsigned int dsa_P_size;
extern unsigned int dsa_Q_size;
extern unsigned int dsa_G_size;

int8_t dsa_check(bch_p H, bch_p r, bch_p s) {
	int8_t res = -1;

#ifdef __DEBUG__
	printf(">> dsa_check\n");
	bch_hprint("H", H);
	bch_hprint("r", r);
	bch_hprint("s", s);
#endif
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
#ifdef __DEBUG__
	else
		printf("<< dsa_check: unable to inverse\n");
#endif

    bch_free(w, u1, u2, v, t0, t1, G, P, Q, pub);

#ifdef __DEBUG__
	printf("<< dsa_check\n");
#endif

	return res;
}
