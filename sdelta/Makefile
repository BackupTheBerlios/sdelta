CC      := gcc
PREFIX  ?= /usr
CFLAGS  ?= -O3 -pipe
#CFLAGS  += -finline-functions
LDFLAGS ?= -Wl,-s

.PHONY: all bsd install clean

all:	sdelta	sdelta2

sdelta2: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -lpthread -o sdelta2 -DSDELTA_2

sdelta: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -lpthread -o sdelta


clean:
	rm -f sdelta

bsd:  *.c *.h
	$(CC) $(CFLAGS) -DUSE_LIBMD *.c $(LDFLAGS) -pthread -ldlmalloc /usr/lib/libmd.a -o sdelta
	$(CC) $(CFLAGS) -DUSE_LIBMD *.c $(LDFLAGS) -pthread -ldlmalloc /usr/lib/libmd.a -o sdelta2 -DSDELTA_2

install: sdelta
	mkdir   -m 0755  -p                     $(PREFIX)/bin
	install -m 0755  sdelta   sdelta2       $(PREFIX)/bin
	install -m 0755  sd       sdreq         $(PREFIX)/bin
	mkdir   -m 0755  -p                     $(PREFIX)/share/sdelta
	install -m 0644  LICENSE  USAGE         $(PREFIX)/share/sdelta
	install -m 0644  README   sdreq_README  $(PREFIX)/share/sdelta
	install -m 0644  sdelta.magic           $(PREFIX)/share/sdelta/magic
