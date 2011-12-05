xor: xor.c mbr_xor_key.S mbr_xor_key
	gcc -g -O0 -fdata-sections -ffunction-sections -Wl,--gc-sections mbr_xor_key.S xor.c -o xor
