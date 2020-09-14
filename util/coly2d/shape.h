#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

typedef enum {
  COLY2D_SHAPE_TYPE_LINE,
  COLY2D_SHAPE_TYPE_RECT,
  COLY2D_SHAPE_TYPE_TRIANGLE,
  COLY2D_SHAPE_TYPE_ELLIPSE,
} coly2d_shape_type_t;

typedef struct {
  coly2d_shape_type_t type;
  vec2_t              size;
  float               angle;
} coly2d_shape_t;

#define coly2d_shape_line(ed, a) ((coly2d_shape_t) {  \
      .type  = COLY2D_SHAPE_TYPE_LINE,  \
      .size  = ed,  \
      .angle = a,  \
    })
#define coly2d_shape_rect(sz, a) ((coly2d_shape_t) {  \
      .type  = COLY2D_SHAPE_TYPE_RECT,  \
      .size  = sz,  \
      .angle = a,  \
    })
#define coly2d_shape_triangle(sz, a) ((coly2d_shape_t) {  \
      .type  = COLY2D_SHAPE_TYPE_TRIANGLE,  \
      .size  = sz,  \
      .angle = a,  \
    })
#define coly2d_shape_ellipse(sz, a) ((coly2d_shape_t) {  \
      .type  = COLY2D_SHAPE_TYPE_ELLIPSE,  \
      .size  = sz,  \
      .angle = a,  \
    })

bool
coly2d_shape_valid(
    const coly2d_shape_t* shape
);

bool
coly2d_shape_hittest_lineseg(
    const coly2d_shape_t* shape,
    const vec2_t*         st,
    const vec2_t*         ed
);
