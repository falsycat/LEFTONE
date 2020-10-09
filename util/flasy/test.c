#undef NDEBUG

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./flasy.h"

#define MAX_   16
#define COUNT_ (512*1024)

int main(int argc, char** argv) {
  assert(2 <= argc && argc <= MAX_);

  flasy_t* srv = flasy_new(COUNT_*sizeof(uint32_t), MAX_);

  for (size_t i = 0; i < (size_t) argc-1; ++i) {
    FILE* fp = flasy_open_file(srv, argv[1+i], true);
    assert(fp != NULL);
    for (uint32_t i = 0; i < COUNT_; ++i) {
      assert(fwrite(&i, sizeof(i), 1, fp) == 1);
    }
    flasy_close_file(srv, fp);
  }

  flasy_delete(srv);  /* join */

  for (size_t i = 0; i < (size_t) argc-1; ++i) {
    FILE* fp = fopen(argv[1+i], "rb");
    assert(fp != NULL);
    for (uint32_t i = 0; i < COUNT_; ++i) {
      uint32_t a;
      assert(fread(&a, sizeof(a), 1, fp) == 1);
      if (a != i) printf("%d %d\n", i, a);
      assert(a == i);
    }
    uint8_t dummy_;
    assert(fread(&dummy_, 1, 1, fp) == 0 && feof(fp) != 0);
    fclose(fp);
  }

  return EXIT_SUCCESS;
}
