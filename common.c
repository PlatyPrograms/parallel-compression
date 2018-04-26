
#include "common.h"




void writeToFile(FILE * file, uint64_t * toWrite){
    
    unsigned char * start = &(*toWrite);

    for(int i = 0; i < 8; ++i){
	fputc(start[0], file);
    }
}
