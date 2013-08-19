# ---- arch ----
GCC				= gcc
GCCARGS			= -c $(CONFIG-DBG-$(CONFIG_DBG)) -m32 -march=i386 -nostdlib -fno-builtin $(DEFINES) $(INCLUDES)


RED=printf "\x1b[31m"
GREEN=printf "\x1b[32m"
YELLOW=printf "\x1b[33m"
CYAN=printf "\x1b[36m"
BLUE=printf "\x1b[34m"
NORMAL=printf "\x1b[0m"
REVERSE=printf "\x1b[7m"

# ---- tools ----
GCC_CMD	= $(YELLOW) && printf "[CC]" && $(NORMAL) && printf " %s " $1 && \
	$(GCC) $(GCCARGS) $1 && \
	$(GREEN) && printf "OK\n" && $(NORMAL)

LD_IMG_CMD	= $(CYAN) && printf "[LD]" && $(NORMAL) && \
	printf " %s " $@ && \
	ld -A i386 -melf_i386 -N -static -Ttext $2 -Map=$@.map $1 -o$@.elf > /dev/null 2>/dev/null && \
	objcopy --only-keep-debug $@.elf $@.dbg && \
	objcopy --strip-debug $@.elf && \
	objcopy -O binary $@.elf $@ && \
	$(GREEN) && printf "OK\n" && $(NORMAL)

LD_LIB_CMD	= $(BLUE) && printf "[LD]" && $(NORMAL); \
	printf " %s " $@ && \
	ld --whole-archive -r $1 -o$@ > /dev/null 2>/dev/null && \
	if [ ! -z "$(TO_LINK)" ]; then echo $(CDIR)/$@ >> $(TO_LINK); fi && \
	$(GREEN) && printf "OK\n" && $(NORMAL)
