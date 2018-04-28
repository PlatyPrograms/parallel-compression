

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef WRITE_BUFF
#define WRITE_BUFF

/**
 * This will handle writing to a given file.
 * You push a writeBuff a uint64_t element 
 * and the given key size and this 
 * buffer with automatically handle 
 * writing your information to the
 * desired file. 
 */


typedef struct{

    FILE * file;
    unsigned int keySize;

    unsigned int currBit;
    uint64_t buff;
    
    unsigned int bitsAtEnd;

}writeBuff;

void initWriteBuff(writeBuff * wBuff, FILE * file, unsigned int keySize, unsigned int bitsAtEnd);
void pushToWriteBuff(writeBuff * wBuff, uint64_t toWrite);
void closeWriteBuff(writeBuff * wBuff);


#endif
