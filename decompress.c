#include <stdio.h>
#include <stdint.h>

#define min(a,b) (((a) < (b)) ? (a) : (b))

uint64_t get(FILE* stream, char* used, char* cur, char size) {
  char left = size;
  char c, mask;
  uint64_t ret = 0;
  printf("cur: %hhx\n", *cur);
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
	ret += *cur & (0xff << *used); // possibly weird
	left -= 8 - *used;
	used = 0;
      }
    }
    else {
      printf("size: %i, left: %i, used: %i\n", size, left, *used);
      // fewer characters in cur than I need
      if ((8 - *used) < left) {
	ret = ret << (8 - *used);
	ret += *cur & (0xff << *used); // possibly weird
	left -= 8 - *used;
	*used = 0;
	*cur = fgetc(stream);
      }
      ret = ret << left;
      ret += *cur & (0xff << (8 - left));
      printf("ret: %llx\n", ret);
      *used = left;
      *cur = *cur << *used;
      left = 0;
    }
  }
  return ret;
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
  fseek(meta, 0, SEEK_END);
  int metaLen = ftell(meta);
  rewind(meta);

  fseek(data, 0, SEEK_END);
  int dataLen = ftell(data);
  rewind(data);

  char pSize = fgetc(data); // bit length of one pane
  char fSize = fgetc(meta); // bit length of one frame

  printf("metaLen: %i | dataLen: %i | pSize: %i | fSize: %i\n", metaLen, dataLen, pSize, fSize);

  char remain = 0;
  char metaCur = fgetc(meta);;

  uint64_t metaChunk = get(meta, &remain, &metaCur, metaLen);

  printf("metaChunk: %llx\n", metaChunk);

  metaChunk = get(meta, &remain, &metaCur, metaLen);

  printf("metaChunk: %llx\n", metaChunk);

  fclose(data);
  fclose(meta);
  fclose(fout);
  return 0;
}
