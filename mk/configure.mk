configure: config.h

# Generated headers
config.h: define_var = echo '\#define $1 $($1)'
config.h: define_cfg = echo '\#define $1'
config.h: makefile.config makefile
	$(Q)echo -n > $@
	$(Q)$(call define_var,LOADER_DESCRIPTOR_ADDRESS) >> $@
	$(Q)$(call define_var,LOADER_CODE_ADDRESS) >> $@
	$(Q)$(call define_var,LOADER_STACK_ADDRESS) >> $@
	$(Q)$(call define_var,KERNEL_REAL_CODE_ADDRESS) >> $@
	$(Q)$(call define_var,IO_BUFFER_ADDRESS) >> $@
	$(Q)$(call define_var,KERNEL_CODE_ADDRESS) >> $@
	$(Q)$(call define_var,LOADER_HEAP_START) >> $@
	$(Q)$(call define_var,LOADER_HEAP_SIZE) >> $@
	$(Q)$(call define_var,LOADER_CODE_LBA) >> $@
	$(Q)$(call define_var,LOADER_DESCRIPTOR_LBA) >> $@
	$(Q)$(call define_var,KERNEL_SETUP_SECTORS) >> $@
	$(Q)$(call define_var,DISK_SECTOR_SIZE) >> $@

