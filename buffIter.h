
#include <inttypes.h>
#include <stdbool.h>

#ifndef BUFF_ITER
#define BUFF_ITER


//Assumes max keySize is <= 64
typedef struct{
    unsigned char * buff;

    //This is measured in bytes
    unsigned long int buffSize; 

    //This is measured in bits
    unsigned long int currBit;
    unsigned long int startBitOffset;
    unsigned long int stepSize;

} buffIter;


void initBuffIter(buffIter * iter, 
		  unsigned char * buffer, 
		  unsigned long int bufferSize,
		  unsigned long int bitStepSize,
		  unsigned long int startBitOffset);

bool iterHasNext(buffIter * iter);
void advance(buffIter * iter, uint64_t * result);
unsigned long int unusedBuffBits(buffIter * iter);

#endif
