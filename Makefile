CC = gcc
CFLAGS = -Wall --pedantic -O3
VPATH = src:obj:lib

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
HEADERS = $(wildcard src/*.h) $(wildcard lib/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) $(patsubst lib/%.c, obj/%.o, $(wildcard lib/*.c))

LIBO = ll.o error.o block.o
LIB = bin/libtfs.so

all: $(EXEC)

lib: $(LIB) obj/$(LIBO)

path:	# changes PATH variable for more pleasant command calls
	sh setup.sh

bin/libtfs.so: obj/libtfs.o
	$(CC) $(CFLAGS) -Ilib -Isrc -shared -o $@ $<

obj/libtfs.o: lib/tfs.c
	$(CC) -fpic -c -o $@ $<

bin/%: obj/%.o $(LIBO) $(LIB)
	$(CC) -Lbin -ltfs -o $@ $< $(OBJECTS)

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -Ilib -o $@ $^

clean:
	rm -f $(OBJECTS)

mrproper: clean
	rm -f $(patsubst %, bin/%, $(EXEC)) lib/$(LIB)
