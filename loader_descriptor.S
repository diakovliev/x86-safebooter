#include "loader.h"

.file "loader_descriptor.S"

.text

.code16gcc

loader_descriptor_magic:
.byte LOADER_MAGIC
loader_version:
.byte VER_MAJ,VER_MID,VER_MIN
loader_sectors_count:
.include "loader.img.size"
kernel_sectors_count:
.include "kernel.img.size"
loader_descriptor_sectors_count:
.include "loader_descriptor.img.size"
loader_env:
.incbin "loader_env"

