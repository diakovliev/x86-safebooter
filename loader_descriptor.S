.file "loader_descriptor.S"

.text

.code16gcc

loader_descriptor_magic:	.byte 0xAA,0x00
loader_version: 			.byte 0x00,0x00,0x01
loader_sectors_count:
.include "loader.img.size"
kernel_sectors_count:
.include "kernel.img.size" 				
loader_env:
.incbin "loader_env"

