//
// This file is part of project
// "x86 ssloader" (c) Dmytro Iakovliev 2010
//

#include "common.h"
#include "txt_display.h"

/* 32 bit C code entry point */
void C_start(void *loader_descriptor_address, void *loader_code_address) 
{
	display_t d0;
	display_t d1;
	
	display_init(&d0, TXT_VIDEO_MEM, 80, 10);
	display_clear(&d0);

	display_init(&d1, TXT_VIDEO_MEM + 80*2*10, 80, 15);
	display_clear(&d1);		

	int i;
	for (i = 0; i < 1500; ++i) {
		if (i%2) 
			display_puts(&d0,"Hello\r\n");
		else
			display_puts(&d1,"\tWorld\r\n");
	}
}

