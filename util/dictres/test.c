#undef NDEBUG

#include <stdio.h>
#include <stdlib.h>

#include "./dictres.h"

static dictres_item_t dictres_[] = {
  {"app_name",
      "dictres-test"},
  {"description",
      "dictres provides a static dictionary type which finds value from key fast."},
  {"bye",
      "see you!"},
};

int main(void) {
  dictres_optimize(dictres_);

  printf("%s\n", dictres_find(dictres_, "app_name"));
  printf("====\n");

  printf("%s\n", dictres_find(dictres_, "description"));
  printf("%s\n", dictres_find(dictres_, "bye"));

  return EXIT_SUCCESS;
}
