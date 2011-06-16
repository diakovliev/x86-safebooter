#include <stdio.h>

#define TEST_STRING_H
#include "string.h"

int main(int argc, char **argv) {
	
	char * tst  = "TEST string";
	char * tst0 = "TEST string 0";
	char * tst1 = "TEST string 1";
	char * tst2 = "TEST string 2";
	char dst[20];

	printf(":: strlen(tst0 = %s) = %d\n", tst0, STRING_H(strlen) (tst0) );
	printf(":: strcmp(tst0 = %s, tst1 = %s) = %d\n", tst0, tst1, STRING_H(strcmp) (tst0, tst1) );
	printf(":: strcmp(tst = %s, tst0 = %s) = %d\n", tst, tst0, STRING_H(strcmp) (tst, tst0) );
	printf(":: strncmp(tst0 = %s, tst1 = %s, %d) = %d\n", tst0, tst1, 12, STRING_H(strncmp) (tst0, tst1, 12) );
	printf(":: strncmp(tst0 = %s, tst1 = %s, %d) = %d\n", tst0, tst1, 13, STRING_H(strncmp) (tst0, tst1, 13) );

	STRING_H(strcpy) (dst, tst0);
	printf(":: dst = %s\n", dst );

	STRING_H(memset) (dst, 0, 20);
	printf(":: dst = %s\n", dst );

	STRING_H(memcpy) (dst, tst0, 4);
	printf(":: dst = %s\n", dst );

	STRING_H(strcpy) (dst, tst0);
	printf(":: dst = %s\n", dst );

	STRING_H(strtok) (" ", dst);
	char *tmp = 0;
	while ( tmp = STRING_H(strtok) (" ", 0) ) {
		STRING_H(strrev) (tmp, STRING_H(strlen) (tmp) );
		printf(":: tmp = %s\n", tmp );
	}
	
	printf(":: i = %d\n", STRING_H(atol) ("9980",10) );
	printf(":: i = %d\n", STRING_H(atol) ("9980",10) );
	printf(":: i = %d\n", STRING_H(atol) ("9980",10) );

	printf(":: i = %X\n", STRING_H(atol) ("ABF",16) );
	printf(":: i = %x\n", STRING_H(atol) ("abf",16) );

	return 0;
}

