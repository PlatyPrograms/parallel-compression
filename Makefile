
CC = gcc

CFLAGS = --std=c99 -O3

TARGETS = compressor

all : ${TARGETS}


compressor : compressor.c
	${CC} ${CFLAGS} -o $@ $^


clean :
	rm -f ${TARGETS}
	rm -f *.o