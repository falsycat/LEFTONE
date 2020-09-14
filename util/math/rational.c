#include "./rational.h"

#include <assert.h>
#include <stddef.h>

#include "./algorithm.h"

bool rational_valid(const rational_t* x) {
  return x != NULL && x->den != 0;
}

void rational_add(rational_t* x, const rational_t* a, const rational_t* b) {
  assert(x != NULL);
  assert(rational_valid(a));
  assert(rational_valid(b));

  const int64_t d = math_int64_lcm(a->den, b->den);

  *x = (typeof(*x)) {
    .num = a->num*(d/a->den) + b->num*(d/b->den),
    .den = d,
  };
}
void rational_addeq(rational_t* x, const rational_t* a) {
  assert(rational_valid(x));
  assert(rational_valid(a));

  const int64_t d = math_int64_lcm(x->den, a->den);
  x->num = x->num*(d/x->den) + a->num*(d/a->den);
  x->den = d;
}

void rational_sub(rational_t* x, const rational_t* a, const rational_t* b) {
  assert(x != NULL);
  assert(rational_valid(a));
  assert(rational_valid(b));

  rational_t c = *b;
  c.num *= -1;
  rational_add(x, a, &c);
}
void rational_subeq(rational_t* x, const rational_t* a) {
  assert(rational_valid(x));
  assert(rational_valid(a));

  rational_t c = *a;
  c.num *= -1;
  rational_addeq(x, &c);
}

void rational_mul(rational_t* x, const rational_t* a, const rational_t* b) {
  assert(x != NULL);
  assert(rational_valid(a));
  assert(rational_valid(b));

  *x = (typeof(*x)) {
    .num = a->num*b->num,
    .den = a->den*b->den,
  };
}
void rational_muleq(rational_t* x, const rational_t* a) {
  assert(rational_valid(x));
  assert(rational_valid(a));

  x->num *= a->num;
  x->den *= a->den;
}

void rational_div(rational_t* x, const rational_t* a, const rational_t* b) {
  assert(x != NULL);
  assert(rational_valid(a));
  assert(rational_valid(b));
  assert(b->num != 0);

  *x = (typeof(*x)) {
    .num = a->num*b->den,
    .den = a->den*b->num,
  };
}
void rational_diveq(rational_t* x, const rational_t* a) {
  assert(rational_valid(x));
  assert(rational_valid(a));
  assert(a->num != 0);

  x->num *= a->den;
  x->den *= a->num;
}

void rational_simplify(rational_t* x) {
  assert(rational_valid(x));

  const int64_t d =
      x->num != 0? math_int64_gcd(MATH_ABS(x->num), MATH_ABS(x->den)): x->den;

  x->num /= d;
  x->den /= d;
}
void rational_normalize(rational_t* x, int64_t den) {
  assert(rational_valid(x));
  assert(den != 0);

  x->num = x->num * den / x->den;
  x->den = den;
}

float rational_calculate(const rational_t* x) {
  assert(rational_valid(x));

  return x->num*1.0f / x->den;
}
