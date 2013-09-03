Q=@

include ../../makefile.config
include ../../mk/defconfig.mk

MODULE=start

include ../../mk/functions.mk

BASE_HEADERS+=../../core/loader.h 
BASE_HEADERS+=../../core/gdt_table.h 
BASE_HEADERS+=../../config.h

INCLUDES += -I$(CDIR)/../../

$(MODULE): gdt_table.o loader_start.o mbr.img 
	$(Q)$(call LD_LIB_CMD, loader_start.o gdt_table.o)

# Base objects
mbr.o: mbr.S ../../mbr_xor_key $(BASE_HEADERS)
	$(Q)$(call GCC_CMD, mbr.S)

loader_start.o: loader_start.S $(BASE_HEADERS)
	$(Q)$(call GCC_CMD, loader_start.S)

gdt_table.o : gdt_table.S
	$(Q)$(call GCC_CMD, gdt_table.S)

# Images
mbr.img: mbr.o
	$(Q)$(GREEN) && printf "Generate MBR image... " && \
	$(LD) -T mbr.ld mbr.o -o mbr.img && printf "OK\n" && $(NORMAL)
#	$(Q)$(call LD_IMG_CMD, mbr.o, $(MBR_CODE_ADDRESS))

clean:
	$(Q)rm -f *.img
	$(Q)rm -f *.elf
	$(Q)rm -f *.dbg
	$(Q)rm -f *.map
	$(Q)rm -f *.o
	$(Q)rm -f *.lib

