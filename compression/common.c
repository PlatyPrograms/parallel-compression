
#include "common.h"


void write64ToFile(FILE * file, uint64_t toWrite){    

    for(int i = 1; i <= 8; ++i){	
	unsigned char chunk = (toWrite >> (64 - i*8)) & 0xFF;
	
	fputc(chunk, file);
    }
}
