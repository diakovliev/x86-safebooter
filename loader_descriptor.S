.file "loader_descriptor.S"

.text

.code16

loader_descriptor_magic:	.byte 0xAA,0x00
loader_version: 			.byte 0x00,0x01
loader_sectors_count:		.byte 0x05

