
CC = gcc

CFLAGS = --std=c99 -O3

TARGETS = compressor

all : ${TARGETS}


compressor : compressor.c
	${CC} ${CFLAGS} -o $@ $^

fullclean :
	make clean
	make clear

clean :
	rm -f ${TARGETS}
	rm -f *.o

clear :
	rm -f *.data
	rm -f *.meta
