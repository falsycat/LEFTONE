#include "./dictres.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/chaos/abchash.h"

static void dictres_calculate_hash_(dictres_item_t* items, size_t length) {
  assert(items != NULL || length == 0);

  for (size_t i = 0; i < length; ++i) {
    dictres_item_t* item = &items[i];

    const size_t len = strlen(item->key);
    if (!chaos_abchash_validate(item->key, len)) {
      fprintf(stderr, "dictress: invalid key, '%s'\n", item->key);
      abort();
    }
    item->hash = chaos_abchash(item->key, len);
  }
}

static void dictres_sort_(dictres_item_t* items, size_t length) {
  assert(items != NULL || length == 0);

  for (size_t i = 0; i < length; ++i) {
    for (size_t j = 1; j < length-i; ++j) {
      if (items[j-1].hash > items[j].hash) {
        /* Speed of swapping can be improved but simple is best. :) */
        const dictres_item_t temp = items[j-1];
        items[j-1] = items[j];
        items[j]   = temp;
      }
    }
  }
}

static void dictres_validate_(dictres_item_t* items, size_t length) {
  assert(items != NULL || length == 0);

  for (size_t i = 1; i < length; ++i) {
    if (items[i-1].hash == items[i].hash) {
      fprintf(stderr,
          "dictres: detected hash collision between '%s' and '%s'\n",
          items[i-1].key, items[i].key);
    }
  }
}

void dictres_optimize_(dictres_item_t* items, size_t length) {
  assert(items != NULL || length == 0);

  dictres_calculate_hash_(items, length);
  dictres_sort_(items, length);
  dictres_validate_(items, length);
}

const char* dictres_find_(
    const dictres_item_t* items, size_t length, const char* key) {
  assert(items != NULL || length == 0);

  const size_t keylen = strlen(key);
  assert(chaos_abchash_validate(key, keylen));

  const uint64_t hash = chaos_abchash(key, keylen);

  size_t l = 0, r = length-1;
  while (l <= r && r < length) {
    const size_t i = (l+r)/2;

    if (items[i].hash == hash) {
      return items[i].value;
    }
    if (items[i].hash < hash) {
      l = i+1;
    } else {
      if (i == 0) break;
      r = i-1;
    }
  }
  fprintf(stderr, "dictres: cannot find key, '%s'\n", key);
  abort();
}
