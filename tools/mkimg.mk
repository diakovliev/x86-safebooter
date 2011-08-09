# Mkimg tool makefile

HEADERS += ../crypt/blowfish.h
HEADERS += ../crypt/sha2.h
HEADERS += ../crypt/bch.h
HEADERS += ../crypt/dsa.h
HEADERS += ../crypt/crypt.h

SOURCES += ../crypt/blowfish_key.S
SOURCES += ../crypt/blowfish.c
SOURCES += ../crypt/sha2.c
SOURCES += ../crypt/bch.c
SOURCES += ../crypt/dsa.c
SOURCES += ../crypt/crypt.c
SOURCES += dsa_key.c
SOURCES += dsa_pkey.c
SOURCES += mkimg.c

OBJECTS += blowfish_key.o
OBJECTS += blowfish.o
OBJECTS += sha2.o
OBJECTS += bch.o
OBJECTS += crypt.o
OBJECTS += dsa.o
OBJECTS += dsa_key.o
OBJECTS += dsa_pkey.o
OBJECTS += mkimg.o

.PHONY: clean prepare

mkimg: compile
	gcc $(OBJECTS) -o mkimg 

prepare:
#	rm -rf ./dsa_*
	if [ ! -f dsa_pkey.c ]; then ./gendsa.sh; fi

compile: prepare $(HEADERS) $(SOURCES)
	cp -f ../blowfish_key ./blowfish_key
	gcc -g -O0 -c -I./../crypt -D__HOST_COMPILE__ $(SOURCES)
	rm -f ./blowfish_key	

clean:
	rm -rf ./*.o
