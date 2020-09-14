#include "./hittest.h"

#include <assert.h>
#include <stdbool.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

bool coly2d_hittest_point_and_rect(
    const vec2_t* pos1, const vec2_t* pos2, const vec2_t* size2) {
  assert(vec2_valid(pos1));
  assert(vec2_valid(pos2));
  assert(vec2_valid(size2));

  vec2_t rpos;
  vec2_sub(&rpos, pos2, pos1);
  return
      MATH_ABS(rpos.x) < size2->x &&
      MATH_ABS(rpos.y) < size2->y;
}

bool coly2d_hittest_point_and_triangle(
    const vec2_t* pos1,
    const vec2_t* pos2_a,
    const vec2_t* pos2_b,
    const vec2_t* pos2_c) {
  assert(vec2_valid(pos1));
  assert(vec2_valid(pos2_a));
  assert(vec2_valid(pos2_b));
  assert(vec2_valid(pos2_c));

  vec2_t v1, v2;

  vec2_sub(&v1, pos1,   pos2_a);
  vec2_sub(&v2, pos2_b, pos2_a);
  float a = vec2_cross(&v1, &v2);

  vec2_sub(&v1, pos1,   pos2_b);
  vec2_sub(&v2, pos2_c, pos2_b);
  if (a * vec2_cross(&v1, &v2) <= 0) return false;

  vec2_sub(&v1, pos1,   pos2_c);
  vec2_sub(&v2, pos2_a, pos2_c);
  return a*vec2_cross(&v1, &v2) > 0;
}

bool coly2d_hittest_lineseg_and_lineseg(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2_st,
    const vec2_t* pos2_ed) {
  assert(vec2_valid(pos1_st));
  assert(vec2_valid(pos1_ed));
  assert(vec2_valid(pos2_st));
  assert(vec2_valid(pos2_ed));

  vec2_t x;
  vec2_sub(&x, pos1_ed, pos1_st);
  vec2_t a;
  vec2_sub(&a, pos2_st, pos1_st);
  vec2_t b;
  vec2_sub(&b, pos2_ed, pos1_st);

  vec2_t y;
  vec2_sub(&y, pos2_ed, pos2_st);
  vec2_t c;
  vec2_sub(&c, pos1_st, pos2_st);
  vec2_t d;
  vec2_sub(&d, pos1_ed, pos2_st);

  return
      vec2_cross(&x, &a) * vec2_cross(&x, &b) < 0 &&
      vec2_cross(&y, &c) * vec2_cross(&y, &d) < 0;
}

bool coly2d_hittest_lineseg_and_rect(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2,
    const vec2_t* size2) {
  assert(vec2_valid(pos1_st));
  assert(vec2_valid(pos1_ed));
  assert(vec2_valid(pos2));
  assert(vec2_valid(size2));

  if (coly2d_hittest_point_and_rect(pos1_st, pos2, size2) ||
      coly2d_hittest_point_and_rect(pos1_ed, pos2, size2)) {
    return true;
  }

  const float left   = pos2->x - size2->x;
  const float right  = pos2->x + size2->x;
  const float top    = pos2->y + size2->y;
  const float bottom = pos2->y - size2->y;

  const vec2_t v1 = vec2(left, top);
  const vec2_t v2 = vec2(left, bottom);
  const vec2_t v3 = vec2(right, bottom);
  const vec2_t v4 = vec2(right, top);

  return
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, &v1, &v2) ||
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, &v2, &v3) ||
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, &v3, &v4) ||
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, &v4, &v1);
}

bool coly2d_hittest_lineseg_and_ellipse(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2,
    const vec2_t* size2) {
  assert(vec2_valid(pos1_st));
  assert(vec2_valid(pos1_ed));
  assert(vec2_valid(pos2));
  assert(vec2_valid(size2));

  vec2_t p2 = *pos2;
  p2.x /= size2->x;
  p2.y /= size2->y;

  vec2_t p1st = *pos1_st;
  p1st.x /= size2->x;
  p1st.y /= size2->y;

  vec2_t c1;
  vec2_sub(&c1, &p2, &p1st);
  if (vec2_pow_length(&c1) < 1) return true;

  vec2_t p1ed = *pos1_ed;
  p1ed.x /= size2->x;
  p1ed.y /= size2->y;

  vec2_t c2;
  vec2_sub(&c2, &p2, &p1ed);
  if (vec2_pow_length(&c2) < 1) return true;

  vec2_t ed;
  vec2_sub(&ed, &p1ed, &p1st);
  const float cross = vec2_cross(&ed, &c1);
  if (MATH_ABS(cross) >= vec2_length(&ed)) return false;

  vec2_t st;
  vec2_sub(&st, &p1st, &p1ed);
  return vec2_dot(&c1, &ed)*vec2_dot(&c2, &st) >= 0;
}

bool coly2d_hittest_lineseg_and_triangle(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2_a,
    const vec2_t* pos2_b,
    const vec2_t* pos2_c) {
  assert(vec2_valid(pos1_st));
  assert(vec2_valid(pos1_ed));
  assert(vec2_valid(pos2_a));
  assert(vec2_valid(pos2_b));
  assert(vec2_valid(pos2_c));

  /* TODO(catfoot): Tomas Moller's algorithm may make this function faster. */

  if (coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, pos2_a, pos2_b) ||
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, pos2_b, pos2_c) ||
      coly2d_hittest_lineseg_and_lineseg(pos1_st, pos1_ed, pos2_c, pos2_a)) {
    return true;
  }

  return
      coly2d_hittest_point_and_triangle(pos1_st, pos2_a, pos2_b, pos2_c) ||
      coly2d_hittest_point_and_triangle(pos1_ed, pos2_a, pos2_b, pos2_c);
}
