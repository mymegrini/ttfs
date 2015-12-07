CC = gcc
CFLAGS = -Wall --pedantic -O3
VPATH = src:obj
EXEC = tfs_create # tfs_partition tfs_format tfs_cp tfs_mv tfs_rm tfs_cat tfs_mkdir ...
LIB = lib/libtfs.so
HEADERS = $(wildcard lib/*.h) $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c)) obj/ll.o obj/error.o

all: bin/$(EXEC)

lib: $(LIB)

path:	# changes PATH variable for more pleasant command calls
	sh setup.sh

obj/ll.o: lib/ll.c lib/error.h
	$(CC) $(CFLAGS) -c -Ilib -lerror -o $@ $<

obj/error.o: lib/error.c
	$(CC) $(CFLAGS) -c -Ilib -o $@ $<

lib/libtfs.so: obj/libtfs.o
	$(CC) $(CFLAGS) -shared -o $@ $<

obj/libtfs.o: lib/tfs.c
	$(CC) -fpic -c -o $@ $<

bin/%: $(OBJECTS) $(LIB)
	$(CC) -Llib -ltfs -o $@ $^

obj/%.o: src/%.c lib/ll.h lib/libtfs.so 
	$(CC) $(CFLAGS) -c -Ilib -lll -lerror -o $@ $<

clean:
	rm -f $(OBJECTS) lib/libtfs.o

mrproper: clean
	rm -f $(patsubst %, bin/%, $(EXEC)) $(LIB)
