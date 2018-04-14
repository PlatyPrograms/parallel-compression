#include <stdio.h>

int main(int argc, int** argv) {
  if (argc != 2) {
    printf("usage: ./decompress [output name]\n");
    return 1;
  }
  FILE* data = fopen("data", r);
  FILE* meta = fopen("meta", r);
  FILE* out = fopen(argv[1], w);

  int dIndex = 6; // bit of data currently operating on
  int mindex = 6; // bit of meta currently operating on
  char pSize = fgetc(data); // bit length of one pane
  char fSize = fgetc(meta); // bit length of one frame
  // reset stream to start
  ungetc(pSize, data);
  ungetc(fSize, meta);

  char mask = 0x3f;
  pSize = pSize && mask;
  fSize = fSize && mask;

  fclose(data);
  fclose(meta);
  fclose(out);
  return 0;
}
