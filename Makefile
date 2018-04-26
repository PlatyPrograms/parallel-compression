
CC = gcc

CFLAGS = -O3 --std=c99

TARGETS = compressor

all : ${TARGETS}


compressor : compressor.o common.o buffIter.o
	${CC} ${CFLAGS} -o $@ $^



compressor.o : compressor.c
	${CC} ${CFLAGS} -o $@ -c $^

commmon.o : common.c
	${CC} ${CFLAGS} -o $@ -c $^

buffIter.o : buffIter.c
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
