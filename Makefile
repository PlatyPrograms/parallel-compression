
CC = gcc
MPICC = mpicc

CFLAGS = -O3 --std=c99 -lm

TARGETS = compress serialcompress decompress serialdecompress

COMPSUBDIR = ./compression/
DECPSUBDIR = ./decompression/

all : ${TARGETS}


compress : $(COMPSUBDIR)compressor.o $(COMPSUBDIR)common.o $(COMPSUBDIR)buffIter.o $(COMPSUBDIR)writeBuff.o $(COMPSUBDIR)u64array.o
	${MPICC} ${CFLAGS} -o compress $^

serialcompress : $(COMPSUBDIR)serialcompressor.c $(COMPSUBDIR)common.o $(COMPSUBDIR)buffIter.o $(COMPSUBDIR)writeBuff.o $(COMPSUBDIR)u64array.o
	${CC} ${CFLAGS} -o serialcompress $^

serialdecompress: $(DECPSUBDIR)decompress.c $(DECPSUBDIR)common.o
	$(CC) $(CFLAGS) $(DECPSUBDIR)decompress.c $(DECPSUBDIR)common.o -o serialdecompress

decompress: $(DECPSUBDIR)mpiDecompress.c $(DECPSUBDIR)mpiCommon.o
	$(MPICC) -o decompress $^ -lm

common.o: $(DECPSUBDIR)common.c $(DECPSUBDIR)common.h
	$(CC) $(CFLAGS) $(DECPSUBDIR)common.c -c

mpiCommon.o: $(DECPSUBDIR)mpiCommon.c $(DECPSUBDIR)mpiCommon.h
	$(MPICC) $(DECPSUBDIR)mpiCommon.c -c



compressor.o : $(COMPSUBDIR)compressor.c
	${MPICC} ${CFLAGS} -o $(COMPSUBDIR)$@ -c $^

commmon.o : $(COMPSUBDIR)common.c
	${CC} ${CFLAGS} -o $(COMPSUBDIR)$@ -c $^

buffIter.o : $(COMPSUBDIR)buffIter.c
	${CC} ${CFLAGS} -o $(COMPSUBDIR)$@ -c $^

writeBuff.o : $(COMPSUBDIR)writeBuff.c
	${CC} ${CFLAGS} -o $(COMPSUBDIR)$@ -c $^

u64array.o : $(COMPSUBDIR)u64array.c
	${CC} ${CFLAGS} -o $(COMPSUBDIR)$@ -c $^


clean :
	rm -f ${TARGETS}
	rm -f *.o
	rm -f *~
	make clearDM

clearDM :
	rm -f *.data
	rm -f *.meta
