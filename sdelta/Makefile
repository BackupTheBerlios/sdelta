CC      := gcc
PREFIX  ?= /usr/bin
CFLAGS  ?= -Os -pipe
CFLAGS  += -finline-functions
LDFLAGS ?= -Wl,-s

.PHONY: all bsd install clean

sdelta: *.c *.h
	 $(CC) $(CFLAGS) $(LDFLAGS) *.c -lmhash -lpthread -o sdelta

all: sdelta

clean:
	rm -f sdelta

bsd:  *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -DUSE_MMAP -lmhash -pthread -ldlmalloc -o sdelta

install: sdelta
	install -m 0755  sdelta          $(PREFIX)/bin
	mkdir   -m 0755  -p              $(PREFIX)/share/sdelta
	install -m 0644  LICENSE  USAGE  $(PREFIX)/share/sdelta
