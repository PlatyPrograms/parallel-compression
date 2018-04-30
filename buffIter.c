
#include <stdio.h>
#include "buffIter.h"
#include "common.h"


//Used to setup a buffer iterator
//Range of bitStepSize = [1,64]
void initBuffIter(buffIter * iter, 
		  unsigned char * buffer, 
		  unsigned long int bufferSize,
		  unsigned long int bitStepSize){

    iter->buff = buffer;
    iter->buffSize = bufferSize;
    iter->stepSize = bitStepSize;
    iter->currBit = 0;
}

void setStartOffset(buffIter * iter, unsigned long int startBitOffset){
    iter->currBit = startBitOffset;
}

bool iterHasNext(buffIter * iter){

    //If we can advance in the bits
    return ((iter->currBit + (iter->stepSize)) <= (iter->buffSize)*8 + iter->stepSize);    
}


void advance(buffIter * iter, uint64_t * result){

    //If we can advance in the bits, then do so. I think I fixed this...
    if((iter->currBit + (iter->stepSize)) <= (iter->buffSize)*8 + (iter->stepSize)){
	
	//Current bit and next bit are guaranteed to be in the same
	//uint64_t because the the keySize is guaranteed to be <= 64

	unsigned char * ptr = &(iter->buff[iter->currBit / 8]);
	uint64_t container = 0; 

	//Now, for the next 8 bytes, copy them into container
	container  = (((uint64_t)(ptr[0])) << (56) );
	container += (((uint64_t)(ptr[1])) << (48) );
	container += (((uint64_t)(ptr[2])) << (40) );
	container += (((uint64_t)(ptr[3])) << (32) );
	container += (((uint64_t)(ptr[4])) << (24) );
	container += (((uint64_t)(ptr[5])) << (16) );
	container += (((uint64_t)(ptr[6])) <<  (8) );
	container += (((uint64_t)(ptr[7])) <<  (0) );

	unsigned int nextBitInCntnr = (iter->currBit % 8) + (iter->stepSize);

	//Shift to the right
	container = (container >> (64 - nextBitInCntnr));

	//Then shift back to the left
	container = (container << (64 - iter->stepSize));

	*result = container;

	iter->currBit += iter->stepSize;
    }
}

unsigned long int unusedBuffBits(buffIter * iter){

    return (8 * iter->buffSize) - (iter->currBit);
}
