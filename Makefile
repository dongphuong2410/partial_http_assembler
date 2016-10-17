CC=gcc
CFLAGS=
LDFLAGS=-lzmq -lpthread

all: mfile msender

test: CFLAGS += -DTESTMODE -ggdb
test: all

debug: CFLAGS += -ggdb
debug: all

mfile : mfile.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

msender : msender.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf mfile
	rm -rf msender
