#ifndef DSA_HEADER
#define DSA_HEADER

#include "bch.h"

#ifdef __HOST_COMPILE__
extern unsigned char dsa_priv[];
extern unsigned int dsa_priv_size;
#endif/*__HOST_COMPILE__*/

extern unsigned char dsa_pub[];
extern unsigned char dsa_P[];
extern unsigned char dsa_Q[];
extern unsigned char dsa_G[];
extern unsigned int dsa_pub_size;
extern unsigned int dsa_P_size;
extern unsigned int dsa_Q_size;
extern unsigned int dsa_G_size;

/* ALU byte length */
#define DSA_SIZE	512

#ifdef __HOST_COMPILE__
void dsa_sign(bch_p sha2, bch_p r, bch_p s, bch_random_p random);
#endif/*__HOST_COMPILE__*/
int8_t dsa_check(bch_p sha2, bch_p r, bch_p s);

#endif /* DSA_HEADER */
