CC=gcc
CFLAGS=-O3

decompress: decompress.c
	$(CC) -o decompress decompress.c

clean:
	rm -f *.o decompress
