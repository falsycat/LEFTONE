#pragma once

#include <stdbool.h>

#include <GL/glew.h>

#include "util/math/vector.h"
#include "util/math/matrix.h"

#include "core/locommon/position.h"

struct loshader_uniblock_t;
typedef struct loshader_uniblock_t loshader_uniblock_t;

typedef struct {
  mat4_t proj;
  mat4_t cam;

  locommon_position_t pos;

  float time;
} loshader_uniblock_param_t;

bool
loshader_uniblock_param_valid(
    const loshader_uniblock_param_t* param
);

loshader_uniblock_t*  /* OWNERSHIP */
loshader_uniblock_new(
    void
);

void
loshader_uniblock_delete(
    loshader_uniblock_t* uni  /* OWNERSHIP */
);

void
loshader_uniblock_update_display_param(
    loshader_uniblock_t* uni,
    const vec2_t*        resolution,
    const vec2_t*        dpi
);

void
loshader_uniblock_update_param(
    loshader_uniblock_t*             uni,
    const loshader_uniblock_param_t* param
);

void
loshader_uniblock_bind(
    const loshader_uniblock_t* uni,
    GLuint                     index
);
