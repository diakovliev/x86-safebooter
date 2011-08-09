#ifndef _SHA_H_
#define _SHA_H_

#include <string.h>
#include <stdint.h>

/* POINTER defines a generic pointer type */
typedef uint8_t *POINTER;

/* UINT4 defines a four byte word */
typedef uint32_t UINT4;

/* BYTE defines a unsigned character */
typedef uint8_t BYTE;

#ifndef TRUE
  #define FALSE	0
  #define TRUE	( !FALSE )
#endif /* TRUE */

/* The structure for storing SHS info */
typedef struct
{
	UINT4 digest[ 5 ];            /* Message digest */
	UINT4 countLo, countHi;       /* 64-bit bit count */
	UINT4 data[ 16 ];             /* SHS data buffer */
	int Endianness;
} SHA_CTX;

/* Message digest functions */

void SHAInit(SHA_CTX *);
void SHAUpdate(SHA_CTX *, BYTE *buffer, int count);
void SHAFinal(BYTE *output, SHA_CTX *);

#endif /* end _SHA_H_ */
