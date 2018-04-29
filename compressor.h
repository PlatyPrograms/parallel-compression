
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include "common.h"
#include "buffIter.h"
#include "writeBuff.h"

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

//"You are on this council but we do not grant you the rank of master."
#define MASTER_RANK 0

void initMetaFile(FILE * metaFile, unsigned int lengthOfRunInBits, unsigned long int numRuns){

    //Write first 48 bits as total number of runs
    for(int i = 1; i <= 6; ++i){
	unsigned char chunk = (numRuns >> (48 - i*8)) & 0xFF;

	fputc(chunk, metaFile);	
    }
   
    //Write next 8 bits as the length of a run in bits
    fputc((int) lengthOfRunInBits, metaFile);
}


void initDataFile(FILE * dataFile, unsigned int keySize){

    //Write first 8 bits as keySize
    fputc(keySize, dataFile);
}


#endif
