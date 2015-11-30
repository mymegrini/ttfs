CC = gcc
CFLAGS = -Wall --pedantic
VPATH = src:obj
EXEC = tfs_create tfs_partition tfs_format # tfs_cp tfs_mv tfs_rm tfs_cat tfs_mkdir ...
LIB = libtfs.so
LIBO = libtfs.o
LIBC = tfs.c
HEADERS = $(wildcard $*.h)
OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))


all: bin/$(EXEC)

lib: bin/$(LIB)

obj/ll.o: lib/ll.c
	$(CC) $(CFLAGS) -o $@ $<

bin/$(LIB): obj/$(LIBO)
	$(CC) $(CFLAGS) -shared -o $@ $<

obj/$(LIBO): lib/$(LIBC)
	$(CC) -fpic -c -o $@ $<

bin/$(EXEC): obj/$(OBJECTS) bin/$(LIB)
	$(CC) -Lbin -ltfs -o $@ $<

obj/%.o: src/%.c src/$(HEADERS)
	$(CC) $(CFLAGS) -c -Isrc -Ilib -o $@ $<

clean:
	rm -f obj/$(OBJECTS)

mrproper: clean
	rm -f bin/$(EXEC) bin/$(LIB)
