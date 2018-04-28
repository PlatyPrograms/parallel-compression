CC=gcc
CFLAGS=-O3

decompress: decompress.c common.o
	$(CC) $(CFLAGS) decompress.c common.o -o decompress

compress: compress.c common.o
	$(CC) $(CFLAGS) compress.c common.o -o compress -lm

common.o: common.c common.h
	$(CC) $(CFLAGS) common.c -c

clean:
	rm -f *.o decompress compress
