
#include <inttypes.h>
#include <stdbool.h>

#ifndef BUFF_ITER
#define BUFF_ITER


//Assumes max keySize is <= 64
typedef struct{
    unsigned char * buff;
    unsigned long int currStep;

    //All of these are measured in bytes
    unsigned long int buffSize; 

    //These are measured in bits
    unsigned long int bitStep;

} buffIter;


void initBuffIter(buffIter * iter, 
		  unsigned char * buffer, 
		  unsigned long int bufferSize,
		  unsigned long int bitStepSize);

bool iterHasNext(buffIter * iter);
void advance(buffIter * iter, uint64_t * result);
unsigned long int unusedBuffBits(buffIter * iter);

#endif
