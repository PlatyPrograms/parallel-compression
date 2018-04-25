
#include <inttypes.h>
#include <stdbool.h>

#ifndef BUFF_ITER
#define BUFF_ITER


//Assumes max keySize is <= 64
typedef struct{
    unsigned char * buff;

    //All of these are measured in bytes
    unsigned long int currBuffPos; 
    unsigned long int buffSize; 

    //These are measured in bits
    unsigned long int unusedBits; 
    unsigned long int bitStep;        

    uint64_t currMicroBuff;
    uint64_t nextMicroBuff;

    //These hold the current bitStep 
    //bits starting at their MSB
    uint64_t curr; 
    uint64_t next;

} buffIter;


void initBuffIter(buffIter * iter, 
		  unsigned char * buffer, 
		  unsigned long int bufferSize,
		  unsigned long int bitStepSize);

bool iterHasNext(buffIter * iter);
void advance(buffIter * iter);
unsigned long int unusedBuffBits(buffIter * iter);

#endif
