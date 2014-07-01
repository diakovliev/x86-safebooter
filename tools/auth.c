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

#include <auth.h>
#include <crypt.h>
#include <dsa_check.h>
#include <dsa_sign.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*********************************************************************************/
void custom_random_init(void) {
	srand(time(NULL));
}

/*********************************************************************************/
bch_data custom_random(bch_data max) {
	bch_data r = rand();
	return (r % max) + (r & 1);
}

/*********************************************************************************/
int main(int argc, char **argv)
{	
	int res = 0;	
	unsigned char *buffer = (unsigned char*)malloc(AUTH_BLOCK_SIZE);
	bch_p dsa_r = dsa_alloc();
	bch_p dsa_s = dsa_alloc();
	unsigned char *sha2_buffer = (unsigned char*)malloc(SHA2_SIZE/8);

	/* Random generator */
	bch_random randrom_gen = {
		.init	= custom_random_init,
		.random	= custom_random,
	};

	/* Init random generator */
	(*randrom_gen.init)();

	/* Randomize block */
	bch_data_p start_block_randomize = (bch_data_p)buffer;
	bch_size i;
	for (i = 0; i < AUTH_BLOCK_SIZE; ++i) {
		*start_block_randomize++ = (*randrom_gen.random)(0xFF);
	}

	/* Reinit random generator */
	(*randrom_gen.init)();

	SHA2_func(sha2_buffer,buffer,AUTH_BLOCK_SIZE);
   	bch_p bch_sha2 = dsa_from_ba((bch_data_p)sha2_buffer, SHA2_SIZE/8);

    dsa_sign(bch_sha2, dsa_r, dsa_s, &randrom_gen);
    if (res = dsa_check(bch_sha2, dsa_r, dsa_s))
	{
		printf("!!! Unable to check block signature !!!\n\r");
		goto finish;
	}

	fwrite((void*)buffer, AUTH_BLOCK_SIZE, 1, stdout);
	fwrite((void*)dsa_r, DSA_SIZE, 1, stdout);
	fwrite((void*)dsa_s, DSA_SIZE, 1, stdout);
	
finish:
	free(buffer);
	dsa_free(bch_sha2);
	dsa_free(dsa_r);
	dsa_free(dsa_s);
	free(sha2_buffer);

	return res;
}

