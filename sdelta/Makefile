CC     := gcc
PREFIX := /usr/bin

sdelta: *.c *.h
	 $(CC) $(CFLAGS) $(LDFLAGS) *.c -lmhash -o sdelta

all: sdelta

clean:
	rm -f sdelta

install: sdelta
	install -m 0755 sdelta /usr/bin
	mkdir   -p             /usr/share/sdelta
	install LICENSE	       /usr/share/sdelta
	install USAGE          /usr/share/sdelta
