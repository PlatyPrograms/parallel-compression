CC=gcc
MPI=mpicc
CFLAGS=-O3

all: decompress serialdecompress

serial: serialdecompress

parallel: decompress

serialdecompress: decompress.c common.o
	$(CC) $(CFLAGS) decompress.c common.o -o ../serialdecompress

decompress: mpiDecompress.c mpiCommon.o
	$(MPI) mpiDecompress.c mpiCommon.o -o ../decompress -lm

common.o: common.c common.h
	$(CC) $(CFLAGS) common.c -c

mpiCommon.o: mpiCommon.c mpiCommon.h
	$(MPI) mpiCommon.c -c

clean:
	rm -f *.o decompress compress mpiDecompress *.meta *.data
