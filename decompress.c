#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

uint64_t get(FILE* stream, unsigned char* used, unsigned char* cur, unsigned char size) {
  char left = size;
  char c, mask;
  uint64_t ret = 0;
  while (left && !feof(stream)) {
    if (left > 8) {
      if (!(*used)) {
	ret = ret << 8;
	ret += *cur;
	*cur = fgetc(stream);
	left -= 8;
      }
      else {
	ret = ret << *used;
	ret += (*cur & (0xff << *used)) >> *used; // possibly weird
	left -= 8 - *used;
	*used = 0;
        *cur = fgetc(stream);
      }
    }
    else {
      // fewer characters in cur than I need
      if ((8 - *used) < left) {
	ret = ret << (8 - *used);
	ret += (*cur & (0xff << *used)) >> *used; // possibly weird
	left -= 8 - *used;
	*used = 0;
	*cur = fgetc(stream);
      }
      ret = ret << left;
      unsigned char toAdd = *cur & (0xff << (8 - left));
      toAdd = toAdd >> (8 - left);
      ret += toAdd;
      *used += left;
      *cur = *cur << left;
      left = 0;
    }
  }
  return ret;
}

/* void put(FILE* stream, uint64_t toPut, unsigned char*used, */
/*          unsigned char* cur, unsigned char size) { */
/*   return; */
/* } */

void put(FILE* stream, uint64_t toPut, unsigned char* used,
         unsigned char* cur, unsigned char size) {
  // writes arbitrary bits to a file
  // used tracks bits filled of cur, cur is the character being accumulated
  // into for writing
  unsigned char left = size; // bits still to write
  unsigned char mask;
  unsigned char slices[8];
  slices[0] = toPut;
  while (left > 0) {
    if (left < (8 - used)) {
      // put remaining bits of left into cur, write if full
    }
    else {
      // fill bottom bits of cur, write
  }
}

void getMetaData(FILE* meta, FILE* data, unsigned char* mUsed,
                 unsigned char* dUsed, unsigned char* mCur,
                 unsigned char* dCur, unsigned char* runLen,
                 unsigned char* keyLen) {
  // assumes a 6-bit run length, key length; can be easily modified
  *mUsed = *dUsed = 0;
  *mCur = fgetc(meta);
  *dCur = fgetc(data);
  unsigned char tagSize = 8;
  *runLen = (char) get(meta, mUsed, mCur, tagSize);
  *keyLen = (char) get(data, dUsed, dCur, tagSize);
}

void decompress(FILE* meta, FILE* data, FILE* out,
                unsigned char* mUsed, unsigned char* dUsed,
                unsigned char* mCur, unsigned char* dCur,
                unsigned char* runLen, unsigned char* keyLen) {
  uint64_t run, key, i, j;
  unsigned char oCur;
  unsigned char oUsed = 0;
  
  while (!feof(meta)) {
    run = get(meta, mUsed, mCur, *runLen);
    // escape code, series of unique keys
    if (run == 0) {
      run = get(meta, mUsed, mCur, *runLen);
      for (j = 0; j < run; ++j) {
        key = get(data, dUsed, dCur, *keyLen);
        put(out, key, &oUsed, &oCur, *keyLen);
      }
    }
    // proper run
    else {
      key = get(data, dUsed, dCur, *keyLen);
      for (j = 0; j < run; ++j) {
        put(out, key, &oUsed, &oCur, *keyLen);
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ./decompress [output name]\n");
    return -1;
  }
  FILE* data = fopen("data", "rb");
  FILE* meta = fopen("meta", "rb");
  FILE* fout = fopen(argv[1], "wb");

  printf("opened %s\n", argv[1]);
  
  unsigned char mUsed, dUsed, mCur, dCur, runLen, keyLen;
  getMetaData(meta, data, &mUsed, &dUsed, &mCur, &dCur, &runLen, &keyLen);

  printf("runLen: %i | keyLen: %i\n", runLen, keyLen);
  
  /* int i; */
  /* for (i = 0; i < 6; ++i) { */
  /*   uint64_t dataChunk = get(data, &dUsed, &dCur, 5); */
  /*   printf("dataChunk: %" PRIx64 "\n", dataChunk); */
  /* } */

  /* uint64_t metaChunk = get(meta, &used, &metaCur, fSize); */

  /* printf("metaChunk: %llx\n", metaChunk); */

  /* metaChunk = get(meta, &used, &metaCur, fSize); */

  /* printf("metaChunk: %llx\n", metaChunk); */

  fclose(data);
  fclose(meta);
  fclose(fout);
  return 0;
}
