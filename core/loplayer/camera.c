#include "./camera.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/matrix.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/locommon/screen.h"
#include "core/locommon/ticker.h"
#include "core/loentity/entity.h"
#include "core/loshader/posteffect.h"

void loplayer_camera_initialize(
    loplayer_camera_t*       camera,
    const locommon_screen_t* screen,
    const locommon_ticker_t* ticker) {
  assert(camera != NULL);
  assert(screen != NULL);
  assert(ticker != NULL);

  mat4_t proj;
  locommon_screen_build_projection_matrix(screen, &proj);

  mat4_t iproj;
  mat4_inv(&iproj, &proj);

  vec4_t v;
  mat4_mul_vec4(&v, &iproj, &vec4(1, 1, 0, 0));

  *camera = (typeof(*camera)) {
    .ticker = ticker,

    .base_brightness = 1,
    .chunk_winsz     = v.xy,

    .pos        = locommon_position(0, 0, vec2(.5f, .5f)),
    .scale      = 1,
    .posteffect = {
      .brightness_whole = 1,
    },
  };
}

void loplayer_camera_deinitialize(loplayer_camera_t* camera) {
  assert(camera != NULL);

}

void loplayer_camera_build_matrix(const loplayer_camera_t* camera, mat4_t* m) {
  assert(camera != NULL);
  assert(m != NULL);

  *m = mat4_scale(camera->scale, camera->scale, 1);
}
