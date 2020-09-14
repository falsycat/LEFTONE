#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct {
  const char* key;
  const char* value;
  uint64_t    hash;
} dictres_item_t;

void
dictres_optimize_(
    dictres_item_t* items,
    size_t          length
);
#define dictres_optimize(items)  \
    dictres_optimize_(items, sizeof(items)/sizeof(items[0]))

const char*
dictres_find_(
    const dictres_item_t* items,
    size_t                length,
    const char*           key
);
#define dictres_find(items, key)  \
    dictres_find_(items, sizeof(items)/sizeof(items[0]), key)
