CC = gcc
CFLAGS = -Wall --pedantic -O3 -std=gnu11
VPATH = src:lib

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
LIB = bin/libutils.so bin/libll.so bin/libtfs.so

HEADERS = $(wildcard lib/*.h) $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) \
$(patsubst lib/%.c, obj/lib%.o, $(wildcard lib/*.c))

LIBLINK = -lll -ltfs -lutils #-lcrypt

all : lib exe

exe : $(EXEC)

lib : $(LIB)

bin/libll.so : obj/liberror.o obj/libblock.o obj/libll.o
	$(CC) -shared -o $@ $^

bin/libtfs.so : obj/libtfsll.o obj/libtfs.o
	$(CC) -shared -o $@ $^ -Lbin -lll -lutils -pthread

bin/libutils.so : obj/libutils.o
	$(CC) -shared -o $@ $^

bin/% : obj/%.o
	$(CC) -o $@ $< -Lbin $(LIBLINK)

obj/lib%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -fpic -c -Ilib -o $@ $<	

obj/%.o : src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -Ilib -o $@ $<

clean :
	rm -f $(OBJECTS)

mrproper: clean
	rm -f $(EXEC) $(LIB)
