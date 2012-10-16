include makefile.config

#-----------------------------------------------------------------------------------
HEADERS+=main/cmd.h
HEADERS+=main/simg.h
HEADERS+=core/common.h
HEADERS+=core/loader_types.h 
HEADERS+=core/string.h 
HEADERS+=core/stdio.h
HEADERS+=core/debug.h
HEADERS+=core/heap.h 
HEADERS+=core/env.h 
HEADERS+=core/time.h 
HEADERS+=tools/lbp.h
HEADERS+=linux/jump_to_kernel.h
HEADERS+=linux/image.h
HEADERS+=crypt/blowfish.h
HEADERS+=crypt/sha1.h
#HEADERS+=crypt/sha2.h
HEADERS+=crypt/bch.h
HEADERS+=crypt/dsa.h
HEADERS+=crypt/crypt.h
HEADERS+=crypt/xor_algos.h
DRIVERS_HEADERS+=drivers/text_display_driver.h
DRIVERS_HEADERS+=drivers/text_display_console.h
DRIVERS_HEADERS+=drivers/keyboard_driver.h
DRIVERS_HEADERS+=drivers/ascii_driver.h
DRIVERS_HEADERS+=drivers/ata_driver.h
DRIVERS_HEADERS+=drivers/serial_driver.h
DRIVERS_HEADERS+=drivers/rtc_driver.h
DRIVERS_HEADERS+=drivers/3rd/ata/mindrvr.h
HEADERS+=$(DRIVERS_HEADERS)

SOURCES+=main/main.c
SOURCES+=main/cmd.c
SOURCES+=main/simg.c
SOURCES+=core/string.c  
SOURCES+=core/heap.c 
SOURCES+=core/stdio.c
SOURCES+=core/env.c 
SOURCES+=linux/jump_to_kernel.S
SOURCES+=linux/image.c
SOURCES+=crypt/blowfish.c
SOURCES+=crypt/blowfish_key.S
SOURCES+=crypt/xor_key.S
SOURCES+=crypt/sha1.c
#SOURCES+=crypt/sha2.c
SOURCES+=crypt/bch.c
SOURCES+=crypt/dsa.c
SOURCES+=crypt/dsa_key.c
#SOURCES+=crypt/dsa_pkey.c
SOURCES+=crypt/crypt.c
SOURCES+=crypt/xor_algos.c
DRIVERS_SOURCES+=drivers/text_display_driver.c
DRIVERS_SOURCES+=drivers/text_display_console.c
DRIVERS_SOURCES+=drivers/keyboard_driver.c
DRIVERS_SOURCES+=drivers/ascii_driver.c
DRIVERS_SOURCES+=drivers/ata_driver.c
DRIVERS_SOURCES+=drivers/serial_driver.c
DRIVERS_SOURCES+=drivers/rtc_driver.c
DRIVERS_SOURCES+=drivers/3rd/ata/mindrvr.c
SOURCES+=$(DRIVERS_SOURCES)

#-----------------------------------------------------------------------------------
CONFIG-DBG-y					:= -ggdb3 -O0 -D__DEBUG__ -Wall
CONFIG-DBG-n					:= -O2
CONFIG-CONSOLE-ENABLED-y 		:= -DCONFIG_CONSOLE_ENABLED
CONFIG-CONSOLE-SERIAL-y 		:= -DCONFIG_CONSOLE_SERIAL
CONFIG-CONSOLE-SERIAL-PORT-COM1 := -DCONFIG_CONSOLE_SERIAL_PORT=COM1
CONFIG-CONSOLE-SERIAL-PORT-COM2 := -DCONFIG_CONSOLE_SERIAL_PORT=COM2
CONFIG-CONSOLE-SERIAL-PORT-COM3 := -DCONFIG_CONSOLE_SERIAL_PORT=COM3
CONFIG-CONSOLE-SERIAL-PORT-COM4 := -DCONFIG_CONSOLE_SERIAL_PORT=COM4
CONFIG-COMMAND-LINE-ENABLED-y	:= -DCONFIG_COMMAND_LINE_ENABLED
CONFIG-RAW-IMAGES-ENABLED-y		:= -DCONFIG_RAW_IMAGES_ENABLED
CONFIG-SIMG-XOR-SCRAMBLED-y		:= -DCONFIG_SIMG_XOR_SCRAMBLED
CONFIG-SIMG-ENCRYPTION-blowfish	:= -DCONFIG_SIMG_BLOWFISH_ENCRYPTED

DEFINES			+= $(CONFIG-CONSOLE-ENABLED-$(CONFIG_CONSOLE_ENABLED))
DEFINES			+= $(CONFIG-CONSOLE-SERIAL-$(CONFIG_CONSOLE_SERIAL))
DEFINES			+= $(CONFIG-CONSOLE-SERIAL-PORT-$(CONFIG_CONSOLE_SERIAL_PORT))
DEFINES			+= $(CONFIG-COMMAND-LINE-ENABLED-$(CONFIG_COMMAND_LINE_ENABLED))
DEFINES			+= $(CONFIG-RAW-IMAGES-ENABLED-$(CONFIG_RAW_IMAGES_ENABLED))
DEFINES			+= $(CONFIG-SIMG-XOR-SCRAMBLED-$(CONFIG_SIMG_XOR_SCRAMBLED))
DEFINES			+= $(CONFIG-SIMG-ENCRYPTION-$(CONFIG_SIMG_ENCRYPTION))

INCLUDES		:= -I./ -I./core -I./linux

#-----------------------------------------------------------------------------------
BASE_HEADERS+=core/loader.h 
BASE_HEADERS+=loader.gen.h
BASE_HEADERS+=core/gdt_table.h 
BASE_HEADERS+=gdt_table.gen.h 

BASE_OBJECTS+=loader_start.o 
BASE_OBJECTS+=gdt_table.o

OBJECTS=$(patsubst %.C,%.o,$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir ${SOURCES}))))

GCCARGS			:= -c $(CONFIG-DBG-$(CONFIG_DBG)) -m32 -march=i386 -nostdlib -fno-builtin $(DEFINES) $(INCLUDES)
GCC				:= gcc

GCC_CMD	= $(GCC) $(GCCARGS)
LD_CMD	= ld -A i386 -melf_i386 -N -static -Ttext $1 -Map=$@.map $^ -o$@.elf && \
objcopy --only-keep-debug $@.elf $@.dbg && \
objcopy --strip-debug $@.elf && \
objcopy -O binary $@.elf $@

default: qemu
.PHONY: loader_gen.h qemu bochs clean

# Tools
mkimg:
	make -C ./tools -e CONFIG_SIMG_XOR_SCRAMBLED=$(CONFIG_SIMG_XOR_SCRAMBLED) CONFIG_SIMG_ENCRYPTION=$(CONFIG_SIMG_ENCRYPTION) -f mkimg.mk

xor:
	make -C ./tools -f xor.mk

# Geometry
loader.img.size: loader.img
	echo ".word `du --apparent-size -B512 $^ | cut -f 1`+1" > $@

loader_env.gen:loader_env
	grep -v "^#" loader_env > $@

loader_descriptor.img.size: loader_env.gen
	echo ".word `du --apparent-size -B512 $^ | cut -f 1`+1" > $@

# Base objects
#mbr.o: $(BASE_HEADERS) xor core/mbr.S mbr_xor_key
#	$(GCC_CMD) $(BASE_HEADERS) core/mbr.S

# Base objects
mbr.o: $(BASE_HEADERS) xor core/mbr_ds.S mbr_xor_key
	$(GCC_CMD) $(BASE_HEADERS) core/mbr_ds.S
	mv mbr_ds.o mbr.o

loader_start.o: core/loader_start.S $(BASE_HEADERS)
	$(GCC_CMD) $<

gdt_table.o : core/gdt_table.S
	$(GCC_CMD) $<

loader_descriptor.o: core/loader_descriptor.S loader.img.size loader_descriptor.img.size 
	$(GCC_CMD) $<

# Objects
${OBJECTS} : makefile.config ${HEADERS} ${SOURCES}
	echo ">>>> ${SOURCES}"
	$(GCC_CMD) $(SOURCES)

# Generated headers
loader.gen.h: define_var = echo '\#define $1 $($1)'
loader.gen.h: define_cfg = echo '\#define $1'
loader.gen.h: makefile.config makefile
	echo -n > $@
	$(call define_var,LOADER_DESCRIPTOR_ADDRESS) >> $@
	$(call define_var,LOADER_CODE_ADDRESS) >> $@
	$(call define_var,LOADER_STACK_ADDRESS) >> $@
	$(call define_var,KERNEL_REAL_CODE_ADDRESS) >> $@
	$(call define_var,IO_BUFFER_ADDRESS) >> $@
	$(call define_var,KERNEL_CODE_ADDRESS) >> $@
	$(call define_var,LOADER_HEAP_START) >> $@
	$(call define_var,LOADER_HEAP_SIZE) >> $@
	$(call define_var,LOADER_CODE_LBA) >> $@
	$(call define_var,LOADER_DESCRIPTOR_LBA) >> $@
	$(call define_var,KERNEL_SETUP_SECTORS) >> $@
	$(call define_var,DISK_SECTOR_SIZE) >> $@

gdt_table.gen.h: define_gdt_entry = echo '\#define $1 0x$(shell nm gdt_table.o | grep $2 | cut -d " " -f 1)'
gdt_table.gen.h: gdt_table.o
	echo -n > $@
	$(call define_gdt_entry,GDT_CODE_SEGMENT,__code) >> $@
	$(call define_gdt_entry,GDT_DATA_SEGMENT,__data) >> $@
	$(call define_gdt_entry,GDT_R_CODE_SEGMENT,__r_code) >> $@
	$(call define_gdt_entry,GDT_R_DATA_SEGMENT,__r_data) >> $@

# Images
mbr.img: mbr.o
	$(call LD_CMD,$(MBR_CODE_ADDRESS))

loader_descriptor.img: loader_descriptor.o
	$(call LD_CMD,$(LOADER_DESCRIPTOR_ADDRESS))
	cp -f loader_descriptor.img loader_descriptor.img.orig 
	./tools/xor loader_descriptor.img s 

loader.img: ${BASE_OBJECTS} ${OBJECTS}
	$(call LD_CMD,$(LOADER_CODE_ADDRESS))
	echo "${OBJECTS}"
	cp -f loader.img loader.img.orig
	./tools/xor loader.img s

# Build
build: mkimg mbr.img loader_descriptor.img loader.img

# Clean
clean:
	make -C ./tools -f mkimg.mk clean
	rm -f ${HDD_IMG}
	rm -f ./drivers/*.gch
	rm -f ./core/*.gch
	rm -f *.gch
	rm -f *.gen
	rm -f *.gen.h
	rm -f *.o.S
	rm -f *.out
	rm -f *.txt
	rm -f *.o
	rm -f *.img.dbg
	rm -f *.img.elf
	rm -f *.img.size
	rm -f *.map
#	rm -f *.img
	rm -f *.simg

# Look http://jamesmcdonald.id.au/faqs/mine/Running_Bochs.html for geometry details.
# Currently used 10MB image.
#$(HDD_IMG): build ${BZIMAGE}
#	dd if=/dev/zero 				of=$@ bs=$(DISK_SECTOR_SIZE) count=20808 && \
#	dd if=mbr.img 					of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc && \
#	dd if=loader_descriptor.img 	of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_DESCRIPTOR_LBA} && \
#	dd if=loader.img 				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_CODE_LBA} && \
#	dd if=${BZIMAGE}				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${KERNEL_CODE_LBA}

kernel.simg kernel.simg.size: ${BZIMAGE}
	./tools/mkimg --verbose -i ${BZIMAGE} -o kernel.simg
	expr `du --apparent-size -B512 kernel.simg | cut -f 1` + 200 > kernel.simg.size

initrd.simg: kernel.simg ${INITRD}
	./tools/mkimg --verbose -i ${INITRD} -o initrd.simg

#$(HDD_IMG): build
$(HDD_IMG): build kernel.simg kernel.simg.size initrd.simg
	dd if=/dev/zero 				of=$@ bs=$(DISK_SECTOR_SIZE) count=208080 && \
	dd if=mbr.img 					of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc && \
	dd if=loader_descriptor.img 	of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_DESCRIPTOR_LBA} && \
	dd if=loader.img 				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_CODE_LBA} && \
	dd if=kernel.simg				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${KERNEL_CODE_LBA} && \
	dd if=initrd.simg				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=$(shell cat kernel.simg.size)
	printf "0x%X\n" ${KERNEL_CODE_LBA} >> env.txt
	printf "0x%X\n" $(shell cat kernel.simg.size) >> env.txt

# Run targets
qemu: PORT=9999
qemu: QEMU_ARGS=-S -gdb tcp::$(PORT) --daemonize
qemu: GDB_ARGS=--symbols=loader.img.dbg --exec loader.img.elf --eval-command="target remote localhost:$(PORT)"
qemu: ${HDD_IMG}
	qemu $(QEMU_ARGS) $<

qemu_gdb: PORT=9999
qemu_gdb: QEMU_ARGS=-S -gdb tcp::$(PORT) --daemonize
qemu_gdb: GDB_ARGS=--symbols=loader.img.dbg --exec loader.img.elf --eval-command="target remote localhost:$(PORT)"
qemu_gdb: ${HDD_IMG}
	qemu $(QEMU_ARGS) $<
	gdb $(GDB_ARGS)

qemu_serial: QEMU_ARGS=-serial stdio -hdb ${HDD_IMG}
qemu_serial: ${HDD_IMG}
	qemu $(QEMU_ARGS) $<

#qemu_serial_usb: QEMU_ARGS=-serial stdio -monitor stdio
qemu_usb: QEMU_ARGS=-monitor stdio
qemu_usb: ${HDD_IMG}
	qemu $(QEMU_ARGS) -usb -drive file=$<,if=none,id=usb-0,boot=on -device usb-storage,drive=usb-0

bochs: ${HDD_IMG}
	bochs -f bochsrc -q

