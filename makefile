include .config

default: qemu
#default: bochs
.PHONY: loader_gen.h qemu clean

#
# Compile using x86_64
#
CFLAGS	= -march=i386 -m32 -nostdlib -O0 -Wa,-R,-aln=$@.S -D__DEBUG_
export CFLAGS_
ASFLAGS	= -march=i386 -m32 -Wl,--oformat=elf32-i386 -D__DEBUG__
export ASFLAGS
LD_CMD	= ld -A i386 -melf_i386 -N -static -Ttext $1 --oformat binary -Map=$@.map $^ -o$@

BASE_HEADERS+=loader.h 
BASE_HEADERS+=loader.gen.h

HEADERS+=$(BASE_HEADERS)
HEADERS+=gdt_table.h 
HEADERS+=gdt_table.gen.h 
HEADERS+=loader_types.h 
HEADERS+=copy_to_upper_memory.h 
HEADERS+=jump_to_kernel.h 
HEADERS+=bios_tools.h 
HEADERS+=console_interface.h 
HEADERS+=string.h 
HEADERS+=lbp.h

SOURCES+=C_loader_start.c 
SOURCES+=bios_tools.c 
SOURCES+=console_interface.c 
SOURCES+=copy_to_upper_memory.S 
SOURCES+=jump_to_kernel.S

OBJECTS+=loader_start.o 
OBJECTS+=gdt_table.o 
OBJECTS+=copy_to_upper_memory.o 
OBJECTS+=jump_to_kernel.o 
OBJECTS+=C_loader_start.o 
OBJECTS+=bios_tools.o 
OBJECTS+=console_interface.o

loader.gen.h: define_var = echo '\#define $1 $($1)'
loader.gen.h: define_cfg = echo '\#define $1'
loader.gen.h: .config makefile
	echo -n > $@
	$(call define_var,COMPILE_PLATFORM__X86_64) >> $@
	$(call define_var,COMPILE_PLATFORM__IA32) >> $@
	$(call define_var,LOADER_DESCRIPTOR_ADDRESS) >> $@
	$(call define_var,LOADER_CODE_ADDRESS) >> $@
	$(call define_var,KERNEL_REAL_CODE_ADDRESS) >> $@
	$(call define_var,IO_BUFFER_ADDRESS) >> $@
	$(call define_var,KERNEL_CODE_ADDRESS) >> $@
	$(call define_var,KERNEL_CODE_OFFSET) >> $@
	$(call define_var,KERNEL_SETUP_SECTORS) >> $@
	$(call define_var,KERNEL_SETUP_ADDRESS) >> $@
	$(call define_var,DISK_SECTOR_SIZE) >> $@
ifeq ($(CONFIG_SUPPORT_CMD_LINE),y) 
	$(call define_cfg,CONFIG_SUPPORT_CMD_LINE) >> $@
endif

gdt_table.o : gdt_table.S $(BASE_HEADERS)

gdt_table.gen.h: define_gdt_entry = echo '\#define $1 0x$(shell nm gdt_table.o | grep $2 | cut -d " " -f 1)'
gdt_table.gen.h: gdt_table.o
	echo -n > $@
	$(call define_gdt_entry,GDT_CODE_SEGMENT,__code) >> $@
	$(call define_gdt_entry,GDT_DATA_SEGMENT,__data) >> $@
	$(call define_gdt_entry,GDT_STACK_SEGMENT,__stack) >> $@
	$(call define_gdt_entry,GDT_R_CODE_SEGMENT,__r_code) >> $@
	$(call define_gdt_entry,GDT_R_DATA_SEGMENT,__r_data) >> $@

mbr.o: mbr.S $(BASE_HEADERS)

mbr.img: mbr.o
	$(call LD_CMD,$(MBR_CODE_ADDRESS))

loader.img: $(OBJECTS)
	$(call LD_CMD,$(LOADER_CODE_ADDRESS))

loader.img.size: loader.img
	echo ".byte `du --apparent-size -B512 $^ | sed s/\s*$^//g`+1" > $@

kernel.img.size: $(BZIMAGE)
	echo ".word `du --apparent-size -B512 $^ | cut -f 1`+1" > $@

loader_descriptor.o: loader.img.size kernel.img.size loader_descriptor.S $(BASE_HEADERS)
loader_start.o: loader_start.S $(BASE_HEADERS)

loader_descriptor.img: loader_descriptor.o
	$(call LD_CMD,$(LOADER_DESCRIPTOR_ADDRESS))

$(OBJECTS): $(SOURCES) $(HEADERS)

# See http://jamesmcdonald.id.au/faqs/mine/Running_Bochs.html for geometry details.
# Currently used 10MB image.
$(HDD_IMG): mbr.img loader_descriptor.img
	dd if=/dev/zero 				of=$@ bs=512 count=20808 && \
	dd if=mbr.img 					of=$@ bs=1 conv=notrunc && \
	dd if=loader_descriptor.img 	of=$@ bs=1 conv=notrunc seek=${LOADER_DESCRIPTOR_OFFSET} && \
	dd if=loader.img 				of=$@ bs=1 conv=notrunc seek=${LOADER_CODE_OFFSET} && \
	dd if=${BZIMAGE}				of=$@ bs=1 conv=notrunc seek=${KERNEL_CODE_OFFSET}

qemu: ${HDD_IMG}
#ifeq ($(CONFIG_SUPPORT_CMD_LINE),y) 
	qemu $<
#else
#	qemu -nographic -serial stdio $<
#endif

bochs: ${HDD_IMG}
	bochs -f bochsrc -q

clean:
	rm -f ${HDD_IMG}
	rm -f *.gen.h
	rm -f *.img
	rm -f *.img.size
	rm -f *.map
	rm -f *.o.S
	rm -f *.out
	rm -f *.txt
	rm -f *.o

