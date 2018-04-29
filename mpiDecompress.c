#include "mpiCommon.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

void getMetaData(FILE* meta, FILE* data, unsigned char* mUsed,
                 unsigned char* dUsed, unsigned char* mCur,
                 unsigned char* dCur, unsigned char* runLen,
                 unsigned char* keyLen, uint64_t* numRuns,
		 unsigned char* expProcs, unsigned char* isTail) {
  // gets data out of the beginning of the meta, data files on the bit
  // lengths of keys and the number of runs
  *mUsed = *dUsed = 0;
  *mCur = fgetc(meta);
  *dCur = fgetc(data);
  unsigned char expProcsSize = 8; // should be 1 bit, but time
  unsigned char tailSize = 8; // should be 1 bit, but time
  unsigned char tagSize = 8; // should be 6 bits, but time
  unsigned char numRunSize = 48;
  
  *expProcs = get(meta, mUsed, mCur, expProcsSize);
  *isTail = get(meta, mUsed, mCur, tailSize);
  *numRuns = get(meta, mUsed, mCur, numRunSize);
  *runLen = (char) get(meta, mUsed, mCur, tagSize);
  *keyLen = (char) get(data, dUsed, dCur, tagSize);
}

void decompress(FILE* meta, FILE* data, char* outBuf,
                unsigned char* mUsed, unsigned char* dUsed,
                unsigned char* mCur, unsigned char* dCur,
                unsigned char runLen, unsigned char keyLen,
                uint64_t numRuns) {
  // iterates throuh meta to find a run length, then through data for that
  // length. continues for numRuns iterations through meta.
  uint64_t run, key, i, j, k, bufIdx;
  unsigned char oCur;
  unsigned char oUsed = 0;
  bufIdx = 0;
  
  for (k = 0; k < numRuns; ++k) {
    run = get(meta, mUsed, mCur, runLen);
    // escape code indicating a series of unique keys
    if (run == 0) {
      run = get(meta, mUsed, mCur, runLen);
        for (j = 0; j < run; ++j) {
        // iterate through unique keys, writing them to the file
        key = get(data, dUsed, dCur, keyLen);
	put(outBuf, key, &oUsed, &oCur, keyLen, &bufIdx);
      }
    }
    // "proper" run (repetition of the same key)
    else {
      key = get(data, dUsed, dCur, keyLen);
        for (j = 0; j < run; ++j) {
        // write the key as many times as the meta file says to
	  put(outBuf, key, &oUsed, &oCur, keyLen, &bufIdx);
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 3) {
    printf("usage: ./decompress [input name] [output name]\n");
    return -1;
  }

  // initialize MPI
  MPI_Init(NULL, NULL);
  int nProc, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &nProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  char rankStr[10];
  int rankLen = sprintf(rankStr, "%i", rank);
  
  printf("my rank: %s | my rankLen: %i\n", rankStr, rankLen);
  
  // get input files
  char* dataName = malloc(strlen(argv[1]) + 5 + rankLen);
  char* metaName = malloc(strlen(argv[1]) + 5 + rankLen);
  memmove(dataName, argv[1], strlen(argv[1]));
  memmove(metaName, argv[1], strlen(argv[1]));
  
  int i;
  for (i = 0; i < rankLen; ++i) {
    dataName[strlen(argv[1]) + i] = rankStr[i];
    metaName[strlen(argv[1]) + i] = rankStr[i];
  }
  dataName[strlen(argv[1]) + rankLen] = '\0';
  metaName[strlen(argv[1]) + rankLen] = '\0';
  
  strncat(dataName, ".data", 5);
  strncat(metaName, ".meta", 5);
  
  printf("data: %s | meta: %s\n", dataName, metaName);
  
  FILE* data = fopen(dataName, "rb");
  FILE* meta = fopen(metaName, "rb");

  // variables for tracking relevant features of files
  uint64_t numRuns;
  unsigned char expProcs, mUsed, dUsed, mCur, dCur, runLen, keyLen,
    isTail;
  
  getMetaData(meta, data, &mUsed, &dUsed, &mCur, &dCur, &runLen, &keyLen,
              &numRuns, &expProcs, &isTail);
  if (nProc < expProcs) {
    printf("ERROR: expected %i processes, got %i\n", expProcs, nProc);
    MPI_Abort(MPI_COMM_WORLD, MPI_ERR_RANK);
  }
  printf("numRuns: %" PRIu64 " | runLen: %u | keyLen: %u\n", numRuns,
  	 runLen, keyLen);
  uint64_t numSlices, run;
  for (i = 0; i < numRuns; ++i) {
    run = get(meta, &mUsed, &mCur, runLen);
    if (run != 0) {
      numSlices += run;
    }
  }
  // reset meta
  fseek(meta, 9, SEEK_SET);
  mUsed = 0;
  mCur = fgetc(meta);
  
  uint64_t numBits = numSlices * runLen;
  uint64_t numBytes = (numBits / 8) + 1;
  char* outBuf = malloc(sizeof(char) * numBytes);
  
  decompress(meta, data, outBuf, &mUsed, &dUsed, &mCur, &dCur, runLen,
  	     keyLen, numRuns);

  if (rank == 0) {
    FILE* out = fopen(argv[2], "wb");
    // write to out file
    fwrite(outBuf, 1, (numBytes-1), out); // -1 because of tail
    int inBytes;
    char* inBuf;
    for (i = 1; i < nProc; ++i) {
      // recieve from i, write data
      MPI_Recv(&inBytes, 1, MPI_INT, i, i, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
      --inBytes;
      inBuf = malloc(sizeof(char) * inBytes);
      MPI_Recv(inBuf, inBytes, MPI_CHAR, i, i, MPI_COMM_WORLD,
	       MPI_STATUS_IGNORE);
      fwrite(inBuf, 1, inBytes, out);
      free(inBuf);
    }
    fclose(out);
  }
  // send decompressed data back to master for writing
  else {
    MPI_Send(&numBytes, 1, MPI_INT, 0, rank, MPI_COMM_WORLD);
    MPI_Send(outBuf, (numBytes-1), MPI_CHAR, 0, rank, MPI_COMM_WORLD);
  }
  
  // tidy up
  free(outBuf);
  
  fclose(data);
  fclose(meta);

  MPI_Finalize();
  return 0;
}

/*  **  **  **  **  **  **  **
    Zzzzz  |\      _,,,--,,_        +-----------------------------+
           /,`.-'`'   ._  \-;;,_    | Why is there always a cat   |
          |,4-  ) )_   .;.(  `'-'   | on whatever you're editing? |
         '---''(_/._)-'(_\_)        +-----------------------------+
**  **  **  **  **  **  **  */
