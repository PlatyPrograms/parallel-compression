
#include "buffIter.h"
#include "common.h"


//Used to setup a buffer iterator
//Range of bitStepSize = [1,64]
void initBuffIter(buffIter * iter, 
		  unsigned char * buffer, 
		  unsigned long int bufferSize,
		  unsigned long int bitStepSize){

    iter->buff = buffer;
    iter->currBuffPos = 0;
    iter->unusedBits = bufferSize*8;
    iter->bitStep = bitStepSize;
    
    iter->buffSize = bufferSize;
    iter->currMicroBuff = iter->buff[0];
    iter->nextMicroBuff = iter->buff[0];
}

bool iterHasNext(buffIter * iter){
    return false;
}


void advance(buffIter * iter){

    //Move the main micro buffer forward
    iter->currMicroBuff = iter->nextMicroBuff;
    
    
    
}

unsigned long int unusedBuffBits(buffIter * iter){

}
