#include "./shape.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "./hittest.h"

bool coly2d_shape_valid(const coly2d_shape_t* shape) {
  return
      shape != NULL &&
      vec2_valid(&shape->size) &&
      MATH_FLOAT_VALID(shape->angle);
}

bool coly2d_shape_hittest_lineseg(
    const coly2d_shape_t* shape, const vec2_t* st, const vec2_t* ed) {
  assert(coly2d_shape_valid(shape));
  assert(vec2_valid(st));
  assert(vec2_valid(ed));

  static const vec2_t origin = vec2(0, 0);

  const float s = -sin(shape->angle);
  const float c = cos(shape->angle);

  const vec2_t st_ = vec2(st->x*c-st->y*s, st->x*s+st->y*c);
  const vec2_t ed_ = vec2(ed->x*c-ed->y*s, ed->x*s+ed->y*c);

  switch (shape->type) {
  case COLY2D_SHAPE_TYPE_LINE:
    return coly2d_hittest_lineseg_and_lineseg(
        &st_, &ed_, &origin, &shape->size);
  case COLY2D_SHAPE_TYPE_RECT:
    return coly2d_hittest_lineseg_and_rect(
        &st_, &ed_, &origin, &shape->size);
  case COLY2D_SHAPE_TYPE_TRIANGLE:
    return coly2d_hittest_lineseg_and_triangle(
        &st_, &ed_,
        &vec2( shape->size.x,  0),
        &vec2(-shape->size.x,  shape->size.y),
        &vec2(-shape->size.x, -shape->size.y));
  case COLY2D_SHAPE_TYPE_ELLIPSE:
    return coly2d_hittest_lineseg_and_ellipse(
        &st_, &ed_, &origin, &shape->size);
  }

  assert(false);
  return  false;
}
