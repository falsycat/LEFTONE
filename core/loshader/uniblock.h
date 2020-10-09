#pragma once

#include <stdbool.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/math/vector.h"
#include "util/math/matrix.h"

#include "core/locommon/position.h"
#include "core/locommon/screen.h"

typedef struct {
  const locommon_screen_t* screen;

  gleasy_buffer_uniform_t buf;
} loshader_uniblock_t;

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

void
loshader_uniblock_initialize(
    loshader_uniblock_t*     uni,
    const locommon_screen_t* screen
);

void
loshader_uniblock_deinitialize(
    loshader_uniblock_t* uni
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
