
CC = gcc
MPICC = mpicc

CFLAGS = -O3 --std=c99 -lm

TARGETS = compressor testmpi

all : ${TARGETS}


compressor : compressor.o common.o buffIter.o writeBuff.o u64array.o
	${MPICC} ${CFLAGS} -o $@ $^

testmpi : testmpi.c
	${MPICC} ${CFLAGS} -o $@ $^


compressor.o : compressor.c
	${MPICC} ${CFLAGS} -o $@ -c $^

commmon.o : common.c
	${CC} ${CFLAGS} -o $@ -c $^

buffIter.o : buffIter.c
	${CC} ${CFLAGS} -o $@ -c $^

writeBuff.o : writeBuff.c
	${CC} ${CFLAGS} -o $@ -c $^

u64array.o : u64array.c
	${CC} ${CFLAGS} -o $@ -c $^


clean :
	rm -f ${TARGETS}
	rm -f *.o
	rm -f *~
	make clearDM

clearDM :
	rm -f *.data
	rm -f *.meta
