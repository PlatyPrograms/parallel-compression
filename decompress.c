#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

uint64_t get(FILE* stream, unsigned char* used, unsigned char* cur,
             unsigned char size) {
  char left = size;
  char c, mask;
  uint64_t ret = 0;
  while (left && !feof(stream)) {
    if (left > 8) {
      if (!(*used)) { // all of cur unused, needed so put it in ret
	ret = ret << 8; // make room for the bits
	ret += *cur; // add cur in
	*cur = fgetc(stream); // new cur
	left -= 8;
      }
      else { // some of cur already used, so only add unused bits
	ret = ret << *used; // get rid of used bits
	ret += (*cur & (0xff << *used)) >> *used; // shift unused bits right,
                                                  // add to ret
	left -= 8 - *used;
	*used = 0;
        *cur = fgetc(stream);
      }
    }
    else {
      // fewer characters in cur than I need
      if ((8 - *used) < left) { // not enough bits in cur, same as l20 clause
	ret = ret << (8 - *used);
	ret += (*cur & (0xff << *used)) >> *used;
	left -= 8 - *used;
	*used = 0;
	*cur = fgetc(stream);
      }
      ret = ret << left; // make room for bits
      unsigned char toAdd = *cur & (0xff << (8 - left)); // get needed bits
      toAdd = toAdd >> (8 - left); // move needed bits to the right
      ret += toAdd; // put 'em in
      *used += left;
      *cur = *cur << left; // get rid of used bits, keep relevant bits left
      left = 0;
    }
  }
  return ret;
}

void put(FILE* stream, uint64_t toPut, unsigned char* used,
         unsigned char* cur, unsigned char size) {
  // writes size number of bits to a stream
  unsigned char left = size; // bits still to write
  uint64_t mask;
  while (left > 0) { // while there are still bits to write
    if (left < (8 - *used)) {
      // if there is enough space in cur for the rest of the bits,
      // put remaining bits of toPut into cur
      mask = 0xffffffffffffffff >> (64 - left); // need rightmost left bits
      *cur = *cur << left; // make room for the bits
      *cur += (char) toPut & mask; // put rightmost left bits into cur
      *used += left; // update bits used by cur
      if (*used == 8) { // if cur is full, write it to the file
        fwrite(cur, 1, 1, stream);
        *cur = 0;
        *used = 0;
      }
      left = 0; // no bits left to write!
    }
    else {
      // need to fill cur, then write and repeat
      uint64_t temp = toPut;
      temp = temp >> (left - (8 - *used)); // move relevant bits tobottom
      mask = 0xffffffffffffffff >> (64 - (8 - *used)); // want rightmost bits
      *cur = *cur << (8 - *used); // make room in cur
      *cur += (char) temp & mask; // put rightmost bits in cur
      fwrite(cur, 1, 1, stream);
      left -= (8 - *used);
      *cur = 0;
      *used = 0;
    }
  }
}

void getMetaData(FILE* meta, FILE* data, unsigned char* mUsed,
                 unsigned char* dUsed, unsigned char* mCur,
                 unsigned char* dCur, unsigned char* runLen,
                 unsigned char* keyLen, uint64_t* numRuns) {
  // gets data out of the beginning of the meta, data files on the bit
  // lengths of keys and the number of runs
  *mUsed = *dUsed = 0;
  *mCur = fgetc(meta);
  *dCur = fgetc(data);
  unsigned char tagSize = 8; // THIS IS INEFFICIENT BUT TIME
  unsigned char numRunSize = 48;
  *numRuns = get(meta, mUsed, mCur, numRunSize);
  *runLen = (char) get(meta, mUsed, mCur, tagSize);
  *keyLen = (char) get(data, dUsed, dCur, tagSize);
}

void decompress(FILE* meta, FILE* data, FILE* out,
                unsigned char* mUsed, unsigned char* dUsed,
                unsigned char* mCur, unsigned char* dCur,
                unsigned char runLen, unsigned char keyLen,
                uint64_t numRuns) {
  // iterates throuh meta to find a run length, then through data for that
  // length. continues for numRuns iterations through meta.
  uint64_t run, key, i, j, k;
  unsigned char oCur;
  unsigned char oUsed = 0;
  
  for (k = 0; k < numRuns; ++k) {
    run = get(meta, mUsed, mCur, runLen);
    // escape code indicating a series of unique keys
    if (run == 0) {
      run = get(meta, mUsed, mCur, runLen);
      ++k;
      for (j = 0; j < run; ++j) {
        // iterate through unique keys, writing them to the file
        key = get(data, dUsed, dCur, keyLen);
        put(out, key, &oUsed, &oCur, keyLen);
      }
    }
    // "proper" run (repetition of the same key)
    else {
      key = get(data, dUsed, dCur, keyLen);
      for (j = 0; j < run; ++j) {
        // write the key as many times as the meta file says to
        put(out, key, &oUsed, &oCur, keyLen);
      }
    }
  }
}

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ./decompress [output name]\n");
    return -1;
  }

  // get input files
  FILE* data = fopen("data", "rb");
  FILE* meta = fopen("meta", "rb");
  FILE* out = fopen(argv[1], "wb");

  // variables for tracking relevant features of files
  uint64_t numRuns;
  unsigned char mUsed, dUsed, mCur, dCur, runLen, keyLen;

  // actually do work
  getMetaData(meta, data, &mUsed, &dUsed, &mCur, &dCur, &runLen, &keyLen,
              &numRuns);
  decompress(meta, data, out, &mUsed, &dUsed, &mCur, &dCur, runLen, keyLen,
             numRuns);

  // tidy up
  fclose(data);
  fclose(meta);
  fclose(out);
  return 0;
}

/*  **  **  **  **  **  **  **
    Zzzzz  |\      _,,,--,,_        +-----------------------------+
           /,`.-'`'   ._  \-;;,_    | Why is there always a cat   |
          |,4-  ) )_   .;.(  `'-'   | on whatever you're editing? |
         '---''(_/._)-'(_\_)        +-----------------------------+
**  **  **  **  **  **  **  */
