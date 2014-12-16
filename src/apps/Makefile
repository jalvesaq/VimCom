CC?=gcc
CFLAGS+=-std=gnu99 -O2 -Wall

all: nvimclient nvimserver libVimR.so

nvimclient: nvimclient.c
	$(CC) $(CFLAGS) nvimclient.c -o nvimclient

nvimserver: nvimserver.c
	$(CC) $(CFLAGS) nvimserver.c -o nvimserver

libVimR.so: vimr.c
	$(CC) -std=gnu99 -fpic -O2 -c vimr.c -o vimr.o
	$(CC) -std=gnu99 -shared -o libVimR.so vimr.o

clean:

