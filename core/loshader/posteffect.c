#include "./posteffect.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/framebuffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./uniblock.h"

/* resources */
#include "core/loshader/anysrc/header.shader.h"
#include "core/loshader/anysrc/posteffect.vshader.h"
#include "core/loshader/anysrc/posteffect.fshader.h"

#define UNIFORM_SRC_ 0

#define PRIMITIVE_COUNT_ 6

#pragma pack(push, 1)
typedef struct {
  float distortion_amnesia;
  float distortion_radial;
  float distortion_urgent;
  float raster_whole;

  float aberration_radial;
  float blur_whole;
  float brightness_whole;
  float fade_radial;
} loshader_posteffect_drawer_internal_param_t;
#pragma pack(pop)

_Static_assert(
    sizeof(float)*8 ==
    sizeof(loshader_posteffect_drawer_internal_param_t),
    "recheck the type has no padding");

void loshader_posteffect_drawer_initialize(
    loshader_posteffect_drawer_t* drawer,
    const loshader_uniblock_t*    uniblock,
    const gleasy_framebuffer_t*   fb) {
  assert(drawer   != NULL);
  assert(uniblock != NULL);
  assert(fb       != NULL);

  *drawer = (typeof(*drawer)) {
    .fb = fb,
  };

  const gleasy_program_t prog = gleasy_program_new(
      loshader_header_shader_,      sizeof(loshader_header_shader_),
      loshader_posteffect_vshader_, sizeof(loshader_posteffect_vshader_),
      loshader_posteffect_fshader_, sizeof(loshader_posteffect_fshader_));

  loshader_single_drawer_initialize(
      &drawer->super,
      prog,
      uniblock,
      sizeof(loshader_posteffect_drawer_internal_param_t));
}

void loshader_posteffect_drawer_set_param(
    loshader_posteffect_drawer_t*             drawer,
    const loshader_posteffect_drawer_param_t* param) {
  assert(drawer != NULL);
  assert(param  != NULL);

  const loshader_posteffect_drawer_internal_param_t p = {
    .distortion_amnesia = param->distortion_amnesia,
    .distortion_radial  = param->distortion_radial,
    .distortion_urgent  = param->distortion_urgent,
    .raster_whole       = param->raster_whole,
    .aberration_radial  = param->aberration_radial,
    .blur_whole         = param->blur_whole,
    .brightness_whole   = param->brightness_whole,
    .fade_radial        = param->fade_radial,
  };
  loshader_single_drawer_set_param(&drawer->super, &p);
}

void loshader_posteffect_drawer_draw(
    const loshader_posteffect_drawer_t* drawer) {
  assert(drawer != NULL);

  glUseProgram(drawer->super.prog);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, drawer->fb->colorbuf);
  glUniform1i(UNIFORM_SRC_, 0);

  loshader_single_drawer_draw_without_use_program(
      &drawer->super, PRIMITIVE_COUNT_);
}
