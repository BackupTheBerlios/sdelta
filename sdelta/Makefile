CC      := gcc
PREFIX  ?= /usr
CFLAGS  ?= -O3 -pipe
#CFLAGS  += -finline-functions
LDFLAGS ?= -Wl,-s

.PHONY: all bsd install clean

sdelta: *.c *.h
	 $(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -lpthread -o sdelta

all: sdelta

clean:
	rm -f sdelta

bsd:  *.c *.h
	$(CC) $(CFLAGS) -DUSE_LIBMD *.c $(LDFLAGS) -pthread -ldlmalloc /usr/lib/libmd.a -o sdelta

install: sdelta
        mkdir   -m 0755  -p              $(PREFIX)/bin
	install -m 0755  sdelta  sd      $(PREFIX)/bin
	mkdir   -m 0755  -p              $(PREFIX)/share/sdelta
	install -m 0644  LICENSE  USAGE  $(PREFIX)/share/sdelta
        install -m 0644  sdelta.magic    $(PREFIX)/share/sdelta/magic
