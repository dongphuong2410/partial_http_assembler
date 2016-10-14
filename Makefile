CC=gcc
CFLAGS=-ggdb
LDFLAGS=-lzmq -lpthread

all: mfile

mfile : mfile.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf mfile
