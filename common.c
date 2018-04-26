
#include "common.h"


void write64ToFile(FILE * file, uint64_t toWrite){
    
    //printf("Writing : " "%" PRIx64 "\n", toWrite);
    //printf("Addr of uint64_t %p\n", &toWrite);
    

    for(int i = 1; i <= 8; ++i){
	
	unsigned char chunk = (toWrite >> (64 - i*8)) & 0xFF;
	//printf("Writing chunk: %x\n", chunk);
	
	fputc(chunk, file);
    }
    //printf("\n");
}
