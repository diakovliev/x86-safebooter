Q=@
# Mkimg tool makefile

CONFIG-DEBUG=y
CONFIG-GPROF-SUPPORT=y
CONFIG-ENABLE-ASM-OUTPUT=n

#------------------------------------------------------------------------
HEADERS += ../crypt/blowfish.h
HEADERS += ../crypt/sha1.h
HEADERS += ../crypt/sha2.h
HEADERS += ../crypt/bch.h
HEADERS += ../crypt/dsa.h
HEADERS += ../crypt/crypt.h
HEADERS += ../crypt/xor_algos.h

SOURCES += blowfish_key.S
SOURCES += xor_key.S
SOURCES += ../crypt/blowfish.c
SOURCES += ../crypt/sha1.c
SOURCES += ../crypt/sha2.c
SOURCES += ../crypt/bch.c
SOURCES += ../crypt/dsa.c
SOURCES += ../crypt/crypt.c
SOURCES += ../crypt/dsa_key.c
SOURCES += ../crypt/dsa_pkey.c
SOURCES += ../crypt/xor_algos.c
SOURCES += mkimg.c

#------------------------------------------------------------------------
.PHONY: clean prepare

CONFIG-GPROF-SUPPORT-y=-pg
CONFIG-GPROF-SUPPORT-n=

CONFIG-DEBUG-y=-g -O0 -D__DEBUG__ -Wall
CONFIG-DEBUG-n=-O2 -fdata-sections -ffunction-sections -Wl,--gc-sections

CONFIG-ENABLE-ASM-OUTPUT-y=-Wa,-a,-ad,-aln=asm.S
CONFIG-ENABLE-ASM-OUTPUT-n=

CONFIG-SIMG-XOR-SCRAMBLED-y		:= -DCONFIG_SIMG_XOR_SCRAMBLED
CONFIG-SIMG-ENCRYPTION-blowfish	:= -DCONFIG_SIMG_BLOWFISH_ENCRYPTED

OBJECTS=$(patsubst %.c,%.o,$(patsubst %.S,%.o,$(notdir $(SOURCES))))

DEFINES	+= $(CONFIG-SIMG-XOR-SCRAMBLED-$(CONFIG_SIMG_XOR_SCRAMBLED))
DEFINES	+= $(CONFIG-SIMG-ENCRYPTION-$(CONFIG_SIMG_ENCRYPTION))

GCC_CMD=gcc $(CONFIG-ENABLE-ASM-OUTPUT-$(CONFIG-ENABLE-ASM-OUTPUT)) $(CONFIG-GPROF-SUPPORT-$(CONFIG-GPROF-SUPPORT)) $(DEFINES)

mkimg: GCC_ARGS=-static
mkimg: compile
	$(Q)$(GCC_CMD) $(GCC_ARGS) $(OBJECTS) -o mkimg 

prepare:
#	rm -rf ../crypt/dsa_*
	$(Q)if [ ! -f ../crypt/dsa_pkey.c ]; then ./gendsa.sh; fi

compile: GCC_ARGS:=$(CONFIG-DEBUG-$(CONFIG-DEBUG)) -c -I./../crypt -D__HOST_COMPILE__
compile: prepare $(HEADERS) $(SOURCES)
	$(Q)$(GCC_CMD) $(GCC_ARGS) $(SOURCES)

clean:
	$(Q)rm -f ./mkimg
	$(Q)rm -f ./*.o
