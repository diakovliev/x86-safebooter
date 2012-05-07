# Xor tool makefile

CONFIG-DEBUG=y
CONFIG-GPROF-SUPPORT=y
CONFIG-ENABLE-ASM-OUTPUT=n

#------------------------------------------------------------------------
HEADERS += ../crypt/xor_algos.h

SOURCES += ../crypt/xor_algos.c
SOURCES += mbr_xor_key.S
SOURCES += xor.c

#------------------------------------------------------------------------
.PHONY: clean prepare

CONFIG-GPROF-SUPPORT-y=-pg
CONFIG-GPROF-SUPPORT-n=

CONFIG-DEBUG-y=-g -O0 -D__DEBUG__ -Wall
CONFIG-DEBUG-n=-O2 -fdata-sections -ffunction-sections -Wl,--gc-sections

CONFIG-ENABLE-ASM-OUTPUT-y=-Wa,-a,-ad,-aln=asm.S
CONFIG-ENABLE-ASM-OUTPUT-n=

OBJECTS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(SOURCES))))

GCC_CMD=gcc $(CONFIG-ENABLE-ASM-OUTPUT-$(CONFIG-ENABLE-ASM-OUTPUT)) $(CONFIG-GPROF-SUPPORT-$(CONFIG-GPROF-SUPPORT)) 

xor: GCC_ARGS=
xor: compile
	$(GCC_CMD) $(GCC_ARGS) $(OBJECTS) -o xor

compile: GCC_ARGS:=$(CONFIG-DEBUG-$(CONFIG-DEBUG)) -c -I./../crypt -D__HOST_COMPILE__
compile: $(HEADERS) $(SOURCES)
	$(GCC_CMD) $(GCC_ARGS) $(SOURCES)

clean:
	rm -rf ./*.o
