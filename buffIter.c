
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
    iter->currStep = 0;
    iter->bitStep = bitStepSize;    
    iter->buffSize = bufferSize;

}

bool iterHasNext(buffIter * iter){

    //Try to get the next set of bits
    unsigned long int currBitInBuff = (iter->currStep)*(iter->bitStep);

    //If we can advance in the bits
    return ((currBitInBuff + (iter->bitStep)) <= (iter->buffSize)*8);
    
}


void advance(buffIter * iter, uint64_t * result){

    //Try to get the next set of bits
    unsigned long int currBitInBuff = (iter->currStep)*(iter->bitStep);

    //If we can advance in the bits, then do so
    if((currBitInBuff + (iter->bitStep)) <= (iter->buffSize)*8){
	
	//Current bit and next bit are guaranteed to be in the same
	//uint64_t because the the keySize is guaranteed to be <= 64

	unsigned char * ptr = &(iter->buff[currBitInBuff / 8]);
	//printf("Address of step: %p\n", ptr);
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

	//printf("Container    " "%" PRIx64 "\n", container);

	unsigned int nextBitInCntnr = (currBitInBuff % 8) + (iter->bitStep);

	//Shift to the right
	container = (container >> (64 - nextBitInCntnr));

	//Then shift back to the left
	container = (container << (64 - iter->bitStep));

	//printf("Putting out: %" PRIx64 "\n\n", container);

	*result = container;

	iter->currStep += 1;
    }
}

unsigned long int unusedBuffBits(buffIter * iter){

    return (8 * iter->buffSize) - (iter->currStep)*(iter->bitStep);
}
