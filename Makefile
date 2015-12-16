CC = gcc
CFLAGS = -Wall --pedantic -O3 -std=c99
VPATH = src:obj:lib

EXEC = $(patsubst src/%.c, bin/%, $(wildcard src/*.c))
LIB = bin/libtfs.so bin/libll.so

OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) \
$(patsubst lib/%.c, obj/%.o, $(wildcard lib/*.c))

all: $(EXEC)

lib: $(LIB)

bin/libtfs.so: lib/tfs.c bin/libll.so $(LIBO)
	$(CC) $(CFLAGS) -fpic -c -Ilib -o obj/tfs.o lib/tfs.c
	$(CC) -shared -Lbin -lll -o bin/libtfs.so obj/tfs.o

bin/libll.so: lib/ll.c
	$(CC) $(CFLAGS) -fpic -c -Ilib -o obj/ll.o lib/ll.c
	$(CC) -shared -o bin/libll.so obj/ll.o

bin/%: obj/%.o $(LIB) $(OBJECTS)
	$(CC) -Lbin -lll -ltfs -o $@ $^

obj/%.o: %.c
	$(CC) $(CFLAGS) -c -Ilib -o $@ $^

clean:
	rm -f $(OBJECTS)

mrproper: clean
	rm -f $(EXEC) $(LIB)
