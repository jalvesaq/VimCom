CC?=gcc

all: libVimR.so

libVimR.so: vimr.c
	$(CC) -std=gnu99 -fPIC -O2 -c vimr.c -o vimr.o
	$(CC) -std=gnu99 -shared -o libVimR.so vimr.o

clean:

