CC     := gcc
PREFIX := /usr/bin

sdelta: *.c *.h
	 $(CC) $(CFLAGS) $(LDFLAGS) *.c -lmhash -o sdelta

all: sdelta

clean:
	rm -f sdelta

bsd:  *.c *.h
	$(CC) $(CFLAGS) $(LDFLAGS) *.c -DUSE_MMAP -lmhash -ldlmalloc -o sdelta

install: sdelta
	install -m 0755 sdelta /usr/bin
	mkdir   -p             /usr/share/sdelta
	install LICENSE	       /usr/share/sdelta
	install USAGE          /usr/share/sdelta
