#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

uint64_t get(FILE* stream, unsigned char* used, unsigned char* cur,
             unsigned char size);

void put(unsigned char* outBuf, uint64_t toPut, unsigned char* used,
	 unsigned char* cur, unsigned char size, uint64_t* bufIdx);
