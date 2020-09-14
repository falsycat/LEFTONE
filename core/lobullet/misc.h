#pragma once

#include <stdbool.h>
#include <stddef.h>

/* dont forget to update EACH macro */
typedef enum {
  LOBULLET_TYPE_LINEAR_LIGHT,
  LOBULLET_TYPE_LINEAR_TRIANGLE,
  LOBULLET_TYPE_BOMB_SQUARE,
  LOBULLET_TYPE_BOMB_TRIANGLE,
} lobullet_type_t;

#define LOBULLET_TYPE_EACH_(PROC) do {  \
  PROC(LINEAR_LIGHT,    linear_light);  \
  PROC(LINEAR_TRIANGLE, linear_triangle);  \
  PROC(BOMB_SQUARE,     bomb_square);  \
  PROC(BOMB_TRIANGLE,   bomb_triangle);  \
} while (0)

const char*
lobullet_type_stringify(
    lobullet_type_t type
);

bool
lobullet_type_unstringify(
    lobullet_type_t* type,
    const char*      v,
    size_t           len
);
