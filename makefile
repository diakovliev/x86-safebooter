#
# Memory map
#
MBR_CODE_ADDRESS			:= 0x7c00
LOADER_DESCRIPTOR_ADDRESS	:= 0x8000
LOADER_CODE_ADDRESS			:= 0x8200

#
# Disk map
#
LOADER_DESCRIPTOR_OFFSET	:=	17408
LOADER_CODE_OFFSET			:=	17920

HDD_IMG						:=	hdd.raw

export BXSHARE				:= /usr/share/bochs

#default: qemu
default: bochs
.PHONY: loader_gen.h qemu clean

#
# Compile using x86_64
#
export CFLAGS				=	-march=i386 -m32 -nostdlib -O0 -Wa,-R,-aln=$@.S
export ASFLAGS				=	-march=i386 -m32 -Wl,--oformat=elf32-i386
LD_CMD						=	ld -A i386 -melf_i386 -N -static -Ttext $1 --oformat binary -Map=$@.map $^ -o$@

BASE_HEADERS				:= loader.h loader.gen.h
HEADERS						:= loader.h loader.gen.h bios_tools.h 
SOURCES						:= C_loader_start.c bios_tools.c 
OBJECTS						:= loader_start.o C_loader_start.o bios_tools.o 

loader.gen.h: define_var 	=	echo '\#define $1 $($1)'
loader.gen.h:
	echo -n > $@
	$(call define_var,LOADER_DESCRIPTOR_ADDRESS) >> $@
	$(call define_var,LOADER_CODE_ADDRESS) >> $@

mbr.o: mbr.S $(BASE_HEADERS)

mbr.img: mbr.o
	$(call LD_CMD,$(MBR_CODE_ADDRESS))

loader_descriptor.o: loader_descriptor.S $(BASE_HEADERS)
loader_start.o: loader_start.S $(BASE_HEADERS)

loader_descriptor.img: loader_descriptor.o
	$(call LD_CMD,$(LOADER_DESCRIPTOR_ADDRESS))

$(OBJECTS): $(SOURCES) $(HEADERS)

loader.img: $(OBJECTS)
	$(call LD_CMD,$(LOADER_CODE_ADDRESS))

# See http://jamesmcdonald.id.au/faqs/mine/Running_Bochs.html for geometry details.
# Currently used 10MB image.
$(HDD_IMG): mbr.img loader_descriptor.img loader.img
	dd if=/dev/zero 				of=$@ bs=512 count=20808 && \
	dd if=mbr.img 					of=$@ bs=1 conv=notrunc && \
	dd if=loader_descriptor.img 	of=$@ bs=1 conv=notrunc seek=${LOADER_DESCRIPTOR_OFFSET} && \
	dd if=loader.img 				of=$@ bs=1 conv=notrunc seek=${LOADER_CODE_OFFSET}

qemu: ${HDD_IMG}
	qemu $<

bochs: ${HDD_IMG}
	bochs -f bochsrc -q

clean:
	rm -f ${HDD_IMG}
	rm -f *.gen.h
	rm -f *.img
	rm -f *.map
	rm -f *.o.S
	rm -f *.out
	rm -f *.txt
	rm -f *.o

