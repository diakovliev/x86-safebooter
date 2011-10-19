#include "loader.h"

.file "loader_descriptor.S"

.text

loader_descriptor_magic:
.byte LOADER_MAGIC
loader_version:
.byte VER_MAJ,VER_MID,VER_MIN
loader_sectors_count:
.include "loader.img.size"
unused:
.word 0
loader_descriptor_sectors_count:
.include "loader_descriptor.img.size"

. = loader_descriptor_magic + DISK_SECTOR_SIZE
/* Environment */
loader_env:
.incbin "loader_env.gen"

