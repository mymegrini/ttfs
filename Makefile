CC = gcc
CFLAGS = -Wall --pedantic -O3 -std=c99
VPATH = src:obj:lib

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
HEADERS = $(wildcard src/*.h) $(wildcard lib/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) $(patsubst lib/%.c, obj/%.o, $(wildcard !lib/tfs.c lib/*.c))

LIBO = obj/ll.o obj/error.o obj/block.o
LIB = bin/libtfs.so

all: $(EXEC)

lib: $(LIB) $(LIBO)

path:	# changes PATH variable for more pleasant command calls
	sh setup.sh

bin/libtfs.so: obj/libtfs.o
	$(CC) $(CFLAGS) -Ilib -Isrc -shared -o $@ $<

obj/libtfs.o: lib/tfs.c
	$(CC) -fpic -c -o $@ $<

bin/%: obj/%.o $(LIBO) $(LIB)
	$(CC) -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -Ilib -o $@ $^

clean:
	rm -f $(OBJECTS) $(LIBO)

mrproper: clean
	rm -f $(EXEC) lib/$(LIB)
