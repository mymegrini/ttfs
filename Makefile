CC = gcc
CFLAGS = -Wall --pedantic -O3 -std=c99
VPATH = src:obj:lib:bin

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
LIB = bin/libll.so bin/libtfs.so

HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) \
$(patsubst lib/%.c, obj/lib%.o, $(wildcard lib/*.c))

LIBLINK = -Llib -lll -ltfs

all : $(EXEC)

lib : $(LIB)

bin/libll.so : obj/libll.o obj/libblock.o obj/liberror.o
	$(CC) -shared -o $@ $^

bin/libtfs.so : obj/libtfs.o
	$(CC) -shared -o $@ $^

bin/% : obj/%.o $(LIB)
	$(CC) -Lbin $(LIBLINK) -o $@ $< $(LIBO)

obj/lib%.o : %.c $(HEADERS)
	$(CC) $(CFLAGS) -fpic -c -Ilib -o $@ $<	

obj/%.o : src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c -Ilib -o $@ $<

clean :
	rm -f $(OBJECTS)

mrproper: clean
	rm -f $(EXEC) $(LIB)
