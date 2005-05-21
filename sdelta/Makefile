CC      := gcc
PREFIX  ?= /usr
CFLAGS  ?= -O3 -pipe
#CFLAGS  += -finline-functions
LDFLAGS ?= -Wl,-s

.PHONY: all bsd install clean

all:	sdelta	sdelta2  sdelta3

sdelta3: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -o sdelta3 -DSDELTA_3

sdelta2: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -o sdelta2 -DSDELTA_2

sdelta: *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -lcrypto -o sdelta  -DSDELTA_1

clean:
	rm -f sdelta

bsd:  *.c *.h
	$(CC) $(CFLAGS) -DSDELTA_1 -DUSE_LIBMD *.c $(LDFLAGS) -ldlmalloc /usr/lib/libmd.a -o sdelta
	$(CC) $(CFLAGS) -DSDELTA_2 -DUSE_LIBMD *.c $(LDFLAGS) -ldlmalloc /usr/lib/libmd.a -o sdelta2
	$(CC) $(CFLAGS) -DSDELTA_3 -DUSE_LIBMD *.c $(LDFLAGS) -ldlmalloc /usr/lib/libmd.a -o sdelta3

install: sdelta
	mkdir   -m 0755  -p                     $(PREFIX)/bin
	install -m 0755  sdelta   sdelta2       $(PREFIX)/bin
	install -m 0755  sdelta3                $(PREFIX)/bin
	install -m 0755  sd       sdreq         $(PREFIX)/bin
	mkdir   -m 0755  -p                     $(PREFIX)/share/sdelta
	install -m 0644  LICENSE  USAGE         $(PREFIX)/share/sdelta
	install -m 0644  README   sdreq_README  $(PREFIX)/share/sdelta
	install -m 0644  sdelta.magic           $(PREFIX)/share/sdelta/magic
