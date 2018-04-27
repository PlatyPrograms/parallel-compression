
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef COMP_COMMON
#define COMP_COMMON

//Additional helper functions for compression

//BUFFER_SIZE measured in bytes
//BUFFER_SIZE must be greater than or equal to 16
#define BUFFER_SIZE 20

//The maximum key size in bits
#define MAX_KEY_SIZE 64


void write64ToFile(FILE * file, uint64_t toWrite);

#endif
