#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int64_t num;
  int64_t den;
} rational_t;

#define rational(n, d) ((rational_t) {  \
    .num = n,  \
    .den = d,  \
  })

bool
rational_valid(
    const rational_t* x
);

void
rational_add(
    rational_t*       x,
    const rational_t* a,
    const rational_t* b
);
void
rational_addeq(
    rational_t*       x,
    const rational_t* a
);

void
rational_sub(
    rational_t*       x,
    const rational_t* a,
    const rational_t* b
);
void
rational_subeq(
    rational_t*       x,
    const rational_t* a
);

void
rational_mul(
    rational_t*       x,
    const rational_t* a,
    const rational_t* b
);
void
rational_muleq(
    rational_t*       x,
    const rational_t* a
);

void
rational_div(
    rational_t*       x,
    const rational_t* a,
    const rational_t* b
);
void
rational_diveq(
    rational_t*       x,
    const rational_t* a
);

void
rational_simplify(
    rational_t* x
);

/* Don't forget that this operation may cause a discrepancy. */
void
rational_normalize(
    rational_t* x,
    int64_t     den
);

float
rational_calculate(
    const rational_t* x
);
