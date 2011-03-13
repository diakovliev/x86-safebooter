#include <stdio.h>

#define TEST_STRING
#include "string.h"

int main(int argc, char **argv) {
	
	char * tst  = "TEST string";
	char * tst0 = "TEST string 0";
	char * tst1 = "TEST string 1";
	char * tst2 = "TEST string 2";
	char dst[20];

	printf(":: strlen(tst0 = %s) = %d\n", tst0, FUNC(strlen) (tst0) );
	printf(":: strcmp(tst0 = %s, tst1 = %s) = %d\n", tst0, tst1, FUNC(strcmp) (tst0, tst1) );
	printf(":: strcmp(tst = %s, tst0 = %s) = %d\n", tst, tst0, FUNC(strcmp) (tst, tst0) );
	printf(":: strncmp(tst0 = %s, tst1 = %s, %d) = %d\n", tst0, tst1, 12, FUNC(strncmp) (tst0, tst1, 12) );
	printf(":: strncmp(tst0 = %s, tst1 = %s, %d) = %d\n", tst0, tst1, 13, FUNC(strncmp) (tst0, tst1, 13) );

	FUNC(strcpy) (dst, tst0);
	printf(":: dst = %s\n", dst );

	FUNC(memset) (dst, 0, 20);
	printf(":: dst = %s\n", dst );

	FUNC(memcpy) (dst, tst0, 4);
	printf(":: dst = %s\n", dst );

	FUNC(strcpy) (dst, tst0);
	printf(":: dst = %s\n", dst );

	FUNC(strtok) (" ", dst);
	char *tmp = 0;
	while ( tmp = FUNC(strtok) (" ", 0) ) {
		FUNC(strrev) (tmp, FUNC(strlen) (tmp) );
		printf(":: tmp = %s\n", tmp );
	}
	
	printf(":: i = %d\n", FUNC(atol) ("9980",10) );
	printf(":: i = %d\n", FUNC(atol) ("9980",10) );
	printf(":: i = %d\n", FUNC(atol) ("9980",10) );

	printf(":: i = %X\n", FUNC(atol) ("ABF",16) );
	printf(":: i = %x\n", FUNC(atol) ("abf",16) );

	return 0;
}

