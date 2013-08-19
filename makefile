Q=@

include makefile.config
include mk/defconfig.mk
include mk/functions.mk

default: build
.PHONY: qemu bochs clean

include mk/configure.mk

# Tools
mkimg:
	$(Q)make -C ./tools -e CONFIG_SIMG_XOR_SCRAMBLED=$(CONFIG_SIMG_XOR_SCRAMBLED) CONFIG_SIMG_ENCRYPTION=$(CONFIG_SIMG_ENCRYPTION) -f mkimg.mk

xor:
	$(Q)make -C ./tools -f xor.mk

# Geometry
loader_env.gen: loader_env
	$(Q)grep -v "^#" loader_env > $@

loader_descriptor.img.size: loader_env.gen
	$(Q)echo ".word `du --apparent-size -B512 $^ | cut -f 1`+1" > $@

loader.img.size: loader.img
	$(Q)echo ".word `du --apparent-size -B512 $^ | cut -f 1`+1" > $@

loader_descriptor.o: INCLUDES += -I$(CDIR)
loader_descriptor.o: INCLUDES += -I$(CDIR)/core
loader_descriptor.o: arch/x86/loader_descriptor.S loader.img.size loader_descriptor.img.size 
	$(Q)$(call GCC_CMD, $<) 

loader_descriptor.img: loader_descriptor.o
	$(Q)$(call LD_IMG_CMD, loader_descriptor.o, $(LOADER_DESCRIPTOR_ADDRESS))
	$(Q)cp -f loader_descriptor.img loader_descriptor.img.orig 
	$(Q)./tools/xor loader_descriptor.img s 

TO_LINK=$(CDIR)/.to_link

include submodules.mk

submodules_build: config.h
	$(Q)$(foreach directory, $(SUBMODULES), make -e TO_LINK=$(TO_LINK) -C $(directory) -f makefile build && )$(GREEN) && printf "** ALL SUBMODULES READY **\n" && $(NORMAL)

submodules_clean:
	$(Q)$(foreach directory, $(SUBMODULES), make -C $(directory) -f makefile clean ; )
	$(Q)rm -f $(TO_LINK)

loader.img: submodules_build $(TO_LINK)
	$(Q)$(call LD_IMG_CMD, $(shell cat $(TO_LINK)), $(LOADER_CODE_ADDRESS))
	$(Q)cp -f loader.img loader.img.orig
	$(Q)./tools/xor loader.img s

# Build
build: xor loader.img loader_descriptor.img

# Clean
clean: submodules_clean
	$(Q)make -C ./tools -f mkimg.mk clean
	$(Q)rm -f config.h
	$(Q)rm -f $(TO_LINK)
	$(Q)rm -f ${HDD_IMG}
	$(Q)rm -f ./drivers/*.gch
	$(Q)rm -f ./core/*.gch
	$(Q)rm -f *.gch
	$(Q)rm -f *.gen
	$(Q)rm -f *.gen.h
	$(Q)rm -f *.o.S
	$(Q)rm -f *.out
	$(Q)rm -f *.txt
	$(Q)rm -f *.o
	$(Q)rm -f *.img.dbg
	$(Q)rm -f *.img.elf
	$(Q)rm -f *.img.size
	$(Q)rm -f *.map
	$(Q)rm -f *.simg

# Look http://jamesmcdonald.id.au/faqs/mine/Running_Bochs.html for geometry details.
# Currently used 10MB image.
#$(HDD_IMG): build ${BZIMAGE}
#	$(Q)dd if=/dev/zero 				of=$@ bs=$(DISK_SECTOR_SIZE) count=20808 && \
#	$(Q)dd if=mbr.img 					of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc && \
#	$(Q)dd if=loader_descriptor.img 	of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_DESCRIPTOR_LBA} && \
#	$(Q)dd if=loader.img 				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_CODE_LBA} && \
#	$(Q)dd if=${BZIMAGE}				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${KERNEL_CODE_LBA}

kernel.simg: mkimg ${BZIMAGE}
	$(Q)./tools/mkimg --verbose -i ${BZIMAGE} -o kernel.simg  

#$(HDD_IMG): build
$(HDD_IMG): build kernel.simg
	$(Q)dd if=/dev/zero 				of=$@ bs=$(DISK_SECTOR_SIZE) count=208080 && \
	dd if=arch/x86/mbr.img 					of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc && \
	dd if=loader_descriptor.img 	of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_DESCRIPTOR_LBA} && \
	dd if=loader.img 				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${LOADER_CODE_LBA} && \
	dd if=kernel.simg				of=$@ bs=$(DISK_SECTOR_SIZE) conv=notrunc seek=${KERNEL_CODE_LBA}

QEMU_CMD=kvm

# Run targets
qemu: PORT=9999
qemu: QEMU_ARGS=-S -gdb tcp::$(PORT) --daemonize -hda
qemu: GDB_ARGS=--symbols=loader.img.dbg --exec loader.img.elf --eval-command="target remote localhost:$(PORT)"
qemu: ${HDD_IMG}
	$(Q)$(QEMU_CMD) $(QEMU_ARGS) $<

qemu_gdb: PORT=9999
qemu_gdb: QEMU_ARGS=-S -gdb tcp::$(PORT) --daemonize -hda
qemu_gdb: GDB_ARGS=--symbols=loader.img.dbg --exec loader.img.elf --eval-command="target remote localhost:$(PORT)"
qemu_gdb: ${HDD_IMG}
	$(Q)$(QEMU_CMD) $(QEMU_ARGS) $<
	$(Q)gdb $(GDB_ARGS)

#qemu_serial: QEMU_ARGS=-serial stdio -nographic
qemu_serial: QEMU_ARGS=-serial stdio -hda
qemu_serial: ${HDD_IMG}
	$(Q)$(QEMU_CMD) $(QEMU_ARGS) $<

bochs: ${HDD_IMG}
	$(Q)bochs -f bochsrc -q

