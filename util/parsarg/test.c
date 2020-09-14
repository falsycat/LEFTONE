#include "./parsarg.h"

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
  parsarg_t pa;
  parsarg_initialize(&pa, argc-1, argv+1);

  while (!parsarg_finished(&pa)) {
    size_t nlen;
    const char* n = parsarg_pop_name(&pa, &nlen);

    char* v;
    parsarg_pop_value(&pa, &v);

    if (n == NULL && v == NULL) continue;
        /* when the args ends with '--' */

    if (n == NULL) {
      printf("param: %s\n", v);
      continue;
    }
    if (nlen == 3 && strncmp(n, "num", nlen) == 0) {
      char* end;
      const intmax_t i = strtoimax(v, &end, 0);
      if (*end == 0 && 0 <= i && i <= 100) {
        printf("num = %"PRIdMAX"\n", i);
      } else {
        printf("error: '%s' is not an valid integer for num (0~100)\n", v);
      }
      continue;
    }
    if (nlen == 3 && strncmp(n, "str", nlen) == 0) {
      printf("str = '%s'\n", v);
      continue;
    }
    printf("error: unknown option '%.*s'\n", (int) nlen, n);
  }

  parsarg_deinitialize(&pa);
}
