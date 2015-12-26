CC = gcc
CFLAGS = -Wall --pedantic -O3 -std=c99
VPATH = src:lib

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
LIB = bin/libll.so bin/libtfs.so bin/libutils.so

HEADERS = $(wildcard lib/*.h) $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) \
$(patsubst lib/%.c, obj/lib%.o, $(wildcard lib/*.c))

LIBLINK = -lll -ltfs -lutils

all : $(EXEC)

exec : $(EXEC)

lib : $(LIB)

bin/libll.so : obj/liberror.o obj/libblock.o obj/libll.o
	$(CC) -shared -o $@ $^

bin/libtfs.so : obj/libtfs.o
	$(CC) -shared -o $@ $^

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
