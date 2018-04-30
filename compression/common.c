#include <sys/time.h>
#include "common.h"


void write64ToFile(FILE * file, uint64_t toWrite){    

    for(int i = 1; i <= 8; ++i){	
	unsigned char chunk = (toWrite >> (64 - i*8)) & 0xFF;
	
	fputc(chunk, file);
    }
}

void subtractTime(struct timeval* start, struct timeval* end,
		  struct timeval* elapsed) {
  elapsed->tv_sec = (end->tv_sec - start->tv_sec);
  if (end->tv_usec < start->tv_usec) {
    --elapsed->tv_sec;
    elapsed->tv_usec = 1000000 + (end->tv_usec - start->tv_usec);
  }
  else {
    elapsed->tv_usec = end->tv_usec - start->tv_usec;
  }
}
