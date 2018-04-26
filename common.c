
#include "common.h"


void write64ToFile(FILE * file, uint64_t * toWrite){
    
    unsigned char * start = &(*toWrite);

    for(int i = 0; i < 8; ++i){
	fputc(*start, file);
	++start;
    }
}
