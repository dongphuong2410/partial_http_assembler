CC=gcc
CFLAGS=
LDFLAGS=-lzmq -lpthread

all: mfile msender

build_test: CFLAGS += -DTESTMODE -ggdb
build_test: all

debug: CFLAGS += -ggdb
debug: all

mfile : mfile.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

msender : msender.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -rf mfile
	rm -rf msender

check: clean build_test
	cd tests && ./test_zeromq
