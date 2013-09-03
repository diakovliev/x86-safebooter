OUTPUT_FORMAT(binary)
SECTIONS {
	.text 0x7c00: {
		mbr.o (.text)
		. = 440;
		FILL(0)
		. = 510;
		SHORT(0xAA55)
	}
}
