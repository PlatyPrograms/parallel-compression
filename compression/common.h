
#include <sys/time.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef COMP_COMMON
#define COMP_COMMON


//Tags used for mpi
#define BUFFER_SIZE_TAG 1
#define SEND_BUFFER_TAG 2

typedef struct{

    uint64_t * data;
    unsigned long int size;
    unsigned long int n;
    uint64_t biggest;

}u64array;


void u64array_init(u64array * arr);
void u64array_get(u64array * arr, unsigned long int idx, uint64_t * ret);
void u64array_push_back(u64array * arr, uint64_t toAdd);
void u64array_clear(u64array * arr);
void u64array_size(u64array * arr, unsigned long int * toPut);


void write64ToFile(FILE * file, uint64_t toWrite);
void subtractTime(struct timeval* start,
		  struct timeval* end,
		  struct timeval* elapsed);


#endif
