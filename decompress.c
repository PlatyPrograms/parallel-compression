#include <stdio.h>
#include <stdint.h>

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

void getMetaData(FILE* meta, FILE* data, unsigned char* mUsed, unsigned char* dUsed,
                 unsigned char* mCur, unsigned char* dCur, unsigned char* runLen,
                 unsigned char* keyLen) {
  // assumes a 6-bit run length, key length; can be easily modified
  *mUsed = *dUsed = 0;
  *mCur = fgetc(meta);
  *dCur = fgetc(data);
  unsigned char tagSize = 8
  *runLen = (char) get(meta, mUsed, mCur, tagSize);
  *keyLen = (char) get(data, dUsed, dCur, tagSize);
}
  

int main(int argc, char** argv) {
  if (argc != 2) {
    printf("usage: ./decompress [output name]\n");
    return 1;
  }
  FILE* data = fopen("data", "rb");
  FILE* meta = fopen("meta", "rb");
  FILE* fout = fopen(argv[1], "wb");

  printf("opened %s\n", argv[1]);

  // find bit lengths of files
  /*
  fseek(meta, 0, SEEK_END);
  int metaLen = ftell(meta);
  rewind(meta);

  fseek(data, 0, SEEK_END);
  int dataLen = ftell(data);
  rewind(data);
  */

  unsigned char mUsed, dUsed, mCur, dCur, runLen, keyLen;
  getMetaData(meta, data, &mUsed, &dUsed, &mCur, &dCur, &runLen, &keyLen);

  printf("runLen: %i | keyLen: %i\n", runLen, keyLen);

  for (int i = 0; i < 6; ++i) {
    uint64_t dataChunk = get(data, &dUsed, &dCur, 5);
    printf("dataChunk: %llx\n", dataChunk);
  }

  /* uint64_t metaChunk = get(meta, &used, &metaCur, fSize); */

  /* printf("metaChunk: %llx\n", metaChunk); */

  /* metaChunk = get(meta, &used, &metaCur, fSize); */

  /* printf("metaChunk: %llx\n", metaChunk); */

  fclose(data);
  fclose(meta);
  fclose(fout);
  return 0;
}
