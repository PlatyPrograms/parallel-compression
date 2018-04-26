
#include "writeBuff.h"
#include "common.h"

void initWriteBuff(writeBuff * wBuff, FILE * file, unsigned int keySize){

    wBuff->file = file;
    wBuff->keySize = keySize;
    wBuff->currBit = 0;
    wBuff->buff = 0;
}

void pushToWriteBuff(writeBuff * wBuff, uint64_t toWrite){
    
    //If we can fit something into our mini-buffer,
    //then do so.
    if(wBuff->currBit + wBuff->keySize <= 64){
	
	//Shift the toWrite elements into place
	//Then add the current buffer and toWrite

	toWrite = toWrite >> wBuff->currBit;
	
	wBuff->buff += toWrite;
	
	wBuff->currBit += wBuff->keySize;
    }

    //Otherwise, we need to fill it as much as
    //possible, then push it, and put the remainder
    //back into the buffer
    else{
	unsigned int avalBits = (64 - wBuff->currBit);

	uint64_t toAdd = toWrite >> wBuff->currBit;
	wBuff->buff += toAdd;

	//Write to file here
	write64ToFile(wBuff->file, &(wBuff->buff));	

	//Reset the buff
	wBuff->buff = 0;

	uint64_t leftoverBits = toWrite << avalBits;
	wBuff->buff += leftoverBits;

	wBuff->currBit = wBuff->keySize - avalBits;
    }
    
}


//Write the last of what we have to the file
void closeBuff(writeBuff *wBuff){

}

