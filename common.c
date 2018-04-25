
#include "common.h"



//Get the first n number of bits from the given 64-bit int
void getFirstNBits(uint64_t * src, 
		   uint64_t * dst, 
		   unsigned int n){

    uint64_t mask = (0xFFFFFFFFFFFFFFFF >> n) << n;
    
    *dst = *src & mask;
}

void getLastNBits(uint64_t * src, 
		  uint64_t * dst, 
		  unsigned int n){

    uint64_t mask = (0xFFFFFFFFFFFFFFFF << (64-n)) >> (64-n);

    *dst = *src & mask;
}

void clearFirstNBits(uint64_t * src, 
		     unsigned int n){

    uint64_t mask = (0xFFFFFFFFFFFFFFFF << n) >> n;

    *src = *src & mask;
}

void clearLastNBits(uint64_t * src, 
		    unsigned int n){

    uint64_t mask = (0xFFFFFFFFFFFFFFFF >> n) << n;

    *src = *src & mask;
}

//Inclusive at idx in first half, idx domain = [0,63]
void joinBits(uint64_t firstHalf, 
	      uint64_t secndHalf, 
	      uint64_t * dst,  
	      unsigned int idx){
    
    clearLastNBits (&firstHalf, idx);
    clearFirstNBits(&secndHalf, (64-idx));

    *dst = firstHalf | secndHalf;
}
