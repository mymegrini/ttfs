CC = gcc
CFLAGS = -Wall --pedantic -O3
VPATH = src:obj
EXEC = tfs_create # tfs_partition tfs_format tfs_cp tfs_mv tfs_rm tfs_cat tfs_mkdir ...
LIB = lib/ll.o lib/libtfs.so
HEADERS = $(wildcard lib/*.h) $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c))

all: bin/$(EXEC)

lib: $(LIB)

path:	# changes PATH variable for more pleasant command calls
	sh setup.h

obj/ll.o: lib/ll.c
	$(CC) $(CFLAGS) -c -Ilib -o $@ $<

lib/libtfs.so: lib/libtfs.o
	$(CC) $(CFLAGS) -shared -o $@ $<

lib/libtfs.o: lib/tfs.c
	$(CC) -fpic -c -o $@ $<

bin/%: obj/%.o 
	$(CC) -Llib -ltfs -o $@ $<

obj/%.o: src/%.c lib/ll.h lib/libtfs.so
	$(CC) $(CFLAGS) -c -Ilib -lll -o $@ $<

clean:
	rm -f $(OBJECTS) lib/ll.o lib/libtfs.o

mrproper: clean
	rm -f $(patsubst %, bin/%, $(EXEC)) $(LIB)
