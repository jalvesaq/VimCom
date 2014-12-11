CC?=gcc
CFLAGS+=-std=gnu99 -O2 -Wall

all: nvimclient nvimserver libvimcom.so

nvimclient: nvimclient.c
	$(CC) $(CFLAGS) nvimclient.c -o nvimclient

nvimserver: nvimserver.c
	$(CC) $(CFLAGS) nvimserver.c -o nvimserver

libvimcom.so: vimcom.c
	$(CC) -std=gnu99 -fpic -O2 -c vimcom.c -o vimcom.o
	$(CC) -std=gnu99 -shared -o libvimcom.so vimcom.o

clean:

