Q=@
# Auth tool makefile

CONFIG-DEBUG=n
CONFIG-GPROF-SUPPORT=y
CONFIG-ENABLE-ASM-OUTPUT=n

#------------------------------------------------------------------------
HEADERS += ../crypt/blowfish/blowfish.h
HEADERS += ../crypt/crypt.h
HEADERS += ../crypt/sha1.h
#HEADERS += ../crypt/sha2.h
HEADERS += ../crypt/bch.h
HEADERS += ../crypt/dsa_base.h
HEADERS += ../crypt/dsa_check.h
HEADERS += ../crypt/private/dsa_sign.h

SOURCES += blowfish_key.S
SOURCES += ../crypt/blowfish/blowfish.c
SOURCES += ../crypt/crypt.c
SOURCES += ../crypt/sha1.c
#SOURCES += ../crypt/sha2.c
SOURCES += ../crypt/bch.c
SOURCES += ../crypt/private/dsa_sign.c
SOURCES += ../crypt/private/dsa_pkey.c
SOURCES += ../crypt/dsa_check.c
SOURCES += ../crypt/dsa_key.c
SOURCES += auth.c

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

auth: GCC_ARGS=-static
auth: compile
	$(Q)$(GCC_CMD) $(GCC_ARGS) $(OBJECTS) -o auth

prepare:
#	rm -rf ../crypt/dsa_*
	$(Q)if [ ! -f ../crypt/private/dsa_pkey.c ]; then ./gendsa.sh; fi

compile: GCC_ARGS:=$(CONFIG-DEBUG-$(CONFIG-DEBUG)) -c -I./../crypt -I./../crypt/blowfish -I./../crypt/private -D__HOST_COMPILE__
compile: prepare $(HEADERS) $(SOURCES)
	$(Q)$(GCC_CMD) $(GCC_ARGS) $(SOURCES)

clean:
	$(Q)rm -f ./auth
	$(Q)rm -f ./*.o
