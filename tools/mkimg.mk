# Mkimg tool makefile

HEADERS += ../crypt/blowfish.h
HEADERS += ../crypt/sha2.h
HEADERS += ../crypt/gmp.h
HEADERS += ../crypt/crypt.h

SOURCES += ../crypt/blowfish_key.S
SOURCES += ../crypt/blowfish.c
SOURCES += ../crypt/sha2.c
SOURCES += ../crypt/gmp.c
SOURCES += ../crypt/crypt.c
SOURCES += mkimg.c

OBJECTS += blowfish_key.o
OBJECTS += blowfish.o
OBJECTS += sha2.o
OBJECTS += gmp.o
OBJECTS += crypt.o
OBJECTS += mkimg.o

.PHONY: clean

mkimg: compile
	gcc $(OBJECTS) -o mkimg 

compile: $(HEADERS) $(SOURCES)
	cp -f ../blowfish_key ./blowfish_key
	gcc -g -O0 -c -D__HOST_COMPILE__ $(SOURCES)
	rm -f ./blowfish_key	
clean:
	rm -rf ./*.o
