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

#define SPT 63
#define HPC 16

/*
 * Translate LBA address to CHS
 */
typedef unsigned char byte_t;
typedef unsigned short word_t;

void lba2chs(unsigned long lba, word_t *C, byte_t *H, byte_t *S, byte_t *data) 
{
	*C = lba / (SPT * HPC);
	*H = (lba / SPT) % HPC;
	*S = (lba % SPT) + 1;
	if (data) {
		data[0] = *H;
		data[1] = ((*C>>8)&0x0003)|(0x3F&*S);
		data[2] = 0x00FF&*C;
	}
}

long nlog2(long x) {
	long r = x;
	asm("bsrl %1,%%eax\n"
		"incl %%eax\n"
		"movl %%eax,%0\n"
		: "=r" (r) : "r" (x) : );
	return r;
}

#include <stdio.h>

long test_data[] = {
	20480
};

void print_chs(int lba)
{
	word_t C = 0;
	byte_t H = 0;
	byte_t S = 0;
	byte_t data[3];
	lba2chs(lba,&C,&H,&S,data);
	printf("LBA:%d\tCHS:%d,%d,%d\tPT:0x%02X,0x%02X,0x%02X\n",lba,C,H,S,data[0],data[1],data[2]);
}

int main()
{
	int i;
	for (i = 0; i < sizeof(test_data)/sizeof(*test_data); ++i) 
		print_chs(test_data[i]);

	unsigned char byte0 = 65;
	unsigned char byte1 = 0;
	asm ( "movb %1,%0" : "=r" (byte1) : "r" (byte0) : "%ah","%bh");
	
	
	printf("%d %d",byte0, byte1);


#define PNLOG2(x) printf("nlog2(%d) = %d, nlg(%d) = %d\n",x,nlog2(x),x,nlog2(x)/nlog2(10))

	PNLOG2(0b00001111);
	PNLOG2(0b00000111);
	PNLOG2(0b00000011);
	PNLOG2(0b00000001);

	PNLOG2(1111);
	PNLOG2(111);
	PNLOG2(11);
	PNLOG2(1);

	return 0;
}

