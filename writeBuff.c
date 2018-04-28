
#include "writeBuff.h"
#include "common.h"

void initWriteBuff(writeBuff * wBuff, FILE * file, unsigned int keySize, unsigned int bitsAtEnd){

    wBuff->file = file;
    wBuff->keySize = keySize;
    wBuff->currBit = 0;
    wBuff->buff = 0;
    wBuff->bitsAtEnd = bitsAtEnd;
    
}

void pushToWriteBuff(writeBuff * wBuff, uint64_t toWrite){
    
    //If we can fit something into our mini-buffer,
    //then do so.
    if(wBuff->currBit + wBuff->keySize <= 64){
	
	//Shift the toWrite elements into place
	//Then add the current buffer and toWrite
	if(wBuff->bitsAtEnd){
	    toWrite = toWrite << wBuff->currBit;
	}
	else{
	    toWrite = toWrite >> wBuff->currBit;
	}
	
	wBuff->buff += toWrite;
	
	wBuff->currBit += wBuff->keySize;
    }

    //Otherwise, we need to fill it as much as
    //possible, then push it, and put the remainder
    //back into the buffer
    else{
	unsigned int avalBits = (64 - wBuff->currBit);

	//printf("Aval bits: %lu \n", avalBits);
	//printf("input to deal with: %" PRIx64 "\n", toWrite);

	if(avalBits != 0){
	    //uint64_t toAdd = (toWrite >> (wBuff->currBit));
	    //printf("Shifted toAdd by %lu bits\n", wBuff->currBit);
	    //printf("toAdd val: %" PRIx64 "\n", toAdd);

	    //wBuff->buff += toAdd;
	    if(wBuff->bitsAtEnd){
		wBuff->buff += (toWrite << (wBuff->currBit));
	    }
	    else{
		wBuff->buff += (toWrite >> (wBuff->currBit));
	    }
	}

	//printf("Calling write function: giving  " "%" PRIx64 "\n", wBuff->buff);

	//Write to file here
	write64ToFile(wBuff->file, (wBuff->buff));	

	//Reset the buff
	wBuff->buff = 0;

	//uint64_t leftoverBits = (toWrite << avalBits);
	//wBuff->buff += leftoverBits;

	if(wBuff->bitsAtEnd){
	    wBuff->buff += (toWrite >> avalBits);
	}
	else{
	    wBuff->buff += (toWrite << avalBits);
	}

	//printf("Next buffer is: %" PRIx64 "\n", wBuff->buff);

	wBuff->currBit = wBuff->keySize - avalBits;
    }
    
}


//Write the last of what we have to the file
void closeWriteBuff(writeBuff *wBuff){

    //We want to take all the bytes we have and write them
    unsigned int numBytesToWrite = (wBuff->currBit / 8) + ((wBuff->currBit % 8) ? 1 : 0);

    for(int i = 1; i <= numBytesToWrite; ++i){
	unsigned char chunk = (wBuff->buff >> (64 - i*8)) & 0xFF;

	fputc(chunk, wBuff->file);	
    }

}

