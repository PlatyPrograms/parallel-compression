#include <stdlib.h>
#include <math.h>
#include "common.h"

void compress(uint64_t* meta, uint64_t* data, FILE* fin,
              unsigned char* inUsed, unsigned char* inCur,
              uint64_t* numRuns, unsigned char* longestRun,
              unsigned char sliceLen, unsigned char numSlices) {
  // writes to arrays for meta, data; writing to file done later
  uint64_t index = 0; // keep track of current index of meta, data
  uint64_t cur, next; // current and next slice
  cur = get(fin, inUsed, inCur, sliceLen);
  uint64_t curRunLen = 0;
  uint64_t i;
  for (i = 0; i < numSlices; ++i) {
    curRunLen = 1;
    next = get(fin, inUsed, inCur, sliceLen);
    while (cur == next && !feof(fin)) { // for the duration of the run
      ++curRunLen;
      ++i;
      next = get(fin, inUsed, inCur, sliceLen);
    }
    meta[index] = curRunLen;
    data[index] = cur;
    ++index;
    ++numRuns;
    if (curRunLen > *longestRun) {
      *longestRun = curRunLen;
    }
    ++(*numRuns);
    cur = next;
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ./compress [input name]\n");
    return 1;
  }

  // open up files
  FILE* data = fopen("data", "wb");
  FILE* meta = fopen("meta", "wb");
  FILE* fin = fopen(argv[1], "rb");

  // variables for tracking relevant features of files
  uint64_t numSlices, runLen;
  unsigned char mUsed, dUsed, mCur, dCur, inUsed, inCur, sliceLen;
  inUsed = inCur = 0;
  mUsed = dUsed = mCur = dCur = 0;

  // get size of input file
  fseek(fin, 0L, SEEK_END);
  uint64_t size = (uint64_t) ftell(fin);
  rewind(fin);
  uint64_t numBits = size * 8;

  sliceLen = 8; // should test every possibility and find most efficient, but time
  numSlices = numBits / sliceLen;

  // initialize variables for compress
  uint64_t* metaArray = malloc(sizeof(uint64_t) * numSlices);
  uint64_t* dataArray = malloc(sizeof(uint64_t) * numSlices);
  uint64_t* compMetaArray = malloc(sizeof(uint64_t) * numSlices); // compressed meta
  uint64_t numKeys, numRuns;
  numKeys = numRuns = 0;
  unsigned char longestRun;

  compress(metaArray, dataArray, fin, &inUsed, &inCur, &numRuns,
           &longestRun, sliceLen, numSlices);
  
  runLen = ceil(log(longestRun) / log(2));
  
  //put(meta, runLen, &mUsed, &mCur, 48);

  uint64_t temp, compIndex, compLen;
  compIndex = compLen = 0;
  uint64_t i;
  for (i = 0; i < numRuns; ++i) {
    // compress meta file
    temp = metaArray[i];
    if (temp != 1) {
      compMetaArray[compIndex] = temp;
      ++compIndex;
    }
    else {
      temp = 1;
      if (compMetaArray[i+1] == 1) {
        ++i;
        while (compMetaArray[i] == 1) {
          ++temp;
          ++i;
        }
        compMetaArray[compIndex] = 0;
        ++compIndex;
        compMetaArray[compIndex] = temp;
        ++compIndex;
        --i; // to preserve the loop
      }
      else {
        compMetaArray[i] = 1;
      }
    }
    ++compLen;
  }
  put(meta, compLen, &mUsed, &mCur, 48);
  printf("hi\n");
  put(meta, runLen, &mUsed, &mCur, 8);
  printf("numRuns: %" PRIu64 "\n", numRuns);
  for (i = 0; i < compLen; ++i) {
    put(meta, compMetaArray[i], &mUsed, &mCur, runLen);
  }
  for(i = 0; i < numRuns; ++i) {
    put(data, dataArray[i], &dUsed, &dCur, sliceLen);
  }

  fclose(meta);
  fclose(data);
  fclose(fin);
  return 0;
}
