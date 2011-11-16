# Mkimg tool makefile

HEADERS += ../crypt/blowfish.h
HEADERS += ../crypt/sha1.h
HEADERS += ../crypt/sha2.h
HEADERS += ../crypt/bch.h
HEADERS += ../crypt/dsa.h
HEADERS += ../crypt/crypt.h

SOURCES += ../crypt/blowfish_key.S
SOURCES += ../crypt/xor_key.S
SOURCES += ../crypt/blowfish.c
SOURCES += ../crypt/sha1.c
SOURCES += ../crypt/sha2.c
SOURCES += ../crypt/bch.c
SOURCES += ../crypt/dsa.c
SOURCES += ../crypt/crypt.c
SOURCES += ../crypt/dsa_key.c
SOURCES += ../crypt/dsa_pkey.c
SOURCES += mkimg.c

OBJECTS += blowfish_key.o
OBJECTS += xor_key.o
OBJECTS += blowfish.o
OBJECTS += sha1.o
OBJECTS += sha2.o
OBJECTS += bch.o
OBJECTS += crypt.o
OBJECTS += dsa.o
OBJECTS += dsa_key.o
OBJECTS += dsa_pkey.o
OBJECTS += mkimg.o

.PHONY: clean prepare

CONFIG-DEBUG=y
CONFIG-GPROF-SUPPORT=y
CONFIG-ENABLE-ASM-OUTPUT=n

CONFIG-GPROF-SUPPORT-y=-pg
CONFIG-GPROF-SUPPORT-n=

CONFIG-DEBUG-y=-g -O0 -D__DEBUG__
CONFIG-DEBUG-n=-O2

CONFIG-ENABLE-ASM-OUTPUT-y=-Wa,-a,-ad,-aln=asm.S
CONFIG-ENABLE-ASM-OUTPUT-n=

GCC_CMD=gcc $(CONFIG-ENABLE-ASM-OUTPUT-$(CONFIG-ENABLE-ASM-OUTPUT)) $(CONFIG-GPROF-SUPPORT-$(CONFIG-GPROF-SUPPORT))

mkimg: GCC_ARGS=
mkimg: compile
	$(GCC_CMD) $(GCC_ARGS) $(OBJECTS) -o mkimg 

prepare:
#	rm -rf ../crypt/dsa_*
	if [ ! -f ../crypt/dsa_pkey.c ]; then ./gendsa.sh; fi

compile: GCC_ARGS:=$(CONFIG-DEBUG-$(CONFIG-DEBUG)) -c -I./../crypt -D__HOST_COMPILE__
compile: prepare $(HEADERS) $(SOURCES)
	$(GCC_CMD) $(GCC_ARGS) $(SOURCES)

clean:
	rm -rf ./*.o
