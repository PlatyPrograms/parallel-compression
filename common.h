
#include <inttypes.h>

#ifndef COMP_COMMON
#define COMP_COMMON

//Additional helper functions for compression

//BUFFER_SIZE measured in bytes
//BUFFER_SIZE must be greater than or equal to 16
#define BUFFER_SIZE 200


typedef struct{

    //TODO

} buffIter;


//Get the frist N bits from the given uint64
void getFirstNBits  (uint64_t * src, uint64_t * dst, unsigned int n);
void getLastNBits   (uint64_t * src, uint64_t * dst, unsigned int n);
void clearFirstNBits(uint64_t * src, unsigned int n);
void clearLastNBits (uint64_t * src, unsigned int n);
void joinBits(uint64_t firstHalf, uint64_t secndHalf, uint64_t * dst, unsigned int idx);


#endif
