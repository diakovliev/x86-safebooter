# ---- arch ----
GCC				= gcc
#GCC				= clang
LD				= ld
AR				= ar
OBJCOPY			= objcopy
GCCARGS			= -c $(CONFIG-DBG-$(CONFIG_DBG)) -m32 -march=i386 -nostdlib -fno-builtin $(DEFINES) $(INCLUDES)

SUPPRESS_LD_OUTPUT= > /dev/null 2>/dev/null
#SUPPRESS_LD_OUTPUT=

# ---- tools ----
define GCC_CMD
	$(BROWN) && printf " [CC]" && $(NORMAL) && printf " %s " $1 && \
	$(GCC) $(GCCARGS) $1 && \
	$(GREEN) && printf "OK\n" && $(NORMAL)
endef

define LD_LIB_CMD 
	$(BLUE) && printf " [LD]" && $(NORMAL) && printf " %s " $@ && \
	$(LD) --whole-archive -r $1 -o$@.lib $(SUPPRESS_LD_OUTPUT) && \
	$(GREEN) && printf "OK\n" && $(NORMAL) && \
	$(if $(filter,$(TO_LINK),),true,if [ ! -z "$(TO_LINK)" ]; then echo $(CDIR)/$@.lib >> $(TO_LINK); fi)
endef

define LD_IMG_CMD 
	$(CYAN) && printf " [LD]" && $(NORMAL) && printf " %s " $@ && \
	$(LD) -A i386 -melf_i386 -N -static -Ttext $2 -Map=$@.map $1 -o$@.elf $(SUPPRESS_LD_OUTPUT) && \
	$(OBJCOPY) --only-keep-debug $@.elf $@.dbg && \
	$(OBJCOPY) --strip-debug $@.elf && \
	$(OBJCOPY) -O binary $@.elf $@ && \
	$(GREEN) && printf "OK\n" && $(NORMAL)
endef

# Common targets
ifneq ($(MODULE),)
default: build

pre-build:
	$(Q)$(GREEN) && printf "** %s **\n" $(MODULE) && $(NORMAL)

post-build:
	$(Q)$(GREEN) && printf "** %s IS READY **\n" $(MODULE) && $(NORMAL)

build: pre-build $(MODULE) post-build
endif
