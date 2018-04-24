
CC = gcc

CFLAGS = --std=c99 -O3

TARGETS = compressor

all : ${TARGETS}


compressor : compressor.o comp_funcs.o
	${CC} ${CFLAGS} -o $@ $^


compressor.o : compressor.c
	${CC} ${CFLAGS} -o $@ -c $^

comp_funcs.o : comp_funcs.c
	${CC} ${CFLAGS} -o $@ -c $^


fullclean :
	make clean
	make clearDM

clean :
	rm -f ${TARGETS}
	rm -f *.o
	rm -f *~

clearDM :
	rm -f *.data
	rm -f *.meta
