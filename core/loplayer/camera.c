#include "./camera.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/algorithm.h"
#include "util/math/matrix.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/easing.h"
#include "core/locommon/position.h"
#include "core/locommon/ticker.h"

#include "./entity.h"
#include "./event.h"
#include "./status.h"

#define LOPLAYER_CAMERA_STATE_EACH_(PROC) do {  \
  PROC(DEFAULT, default);  \
  PROC(COMBAT,  combat);  \
  PROC(DEAD,    dead);  \
  PROC(MENU,    menu);  \
} while (0)

static void loplayer_camera_bind_position_in_area_(
    const loplayer_camera_t*   camera,
    locommon_position_t*       pos,
    const locommon_position_t* areapos,
    const vec2_t*              areasize) {
  assert(camera != NULL);
  assert(locommon_position_valid(pos));
  assert(locommon_position_valid(areapos));
  assert(vec2_valid(areasize));

  vec2_t szoffset = camera->display_chunksz;
  vec2_diveq(&szoffset, camera->scale);

  vec2_t sz;
  vec2_sub(&sz, areasize, &szoffset);

  vec2_t v;
  locommon_position_sub(&v, pos, areapos);

# define fix_coordinate_(axis) do {  \
    if (sz.axis > 0) {  \
      if (MATH_ABS(v.axis) > sz.axis) v.axis = MATH_SIGN(v.axis)*sz.axis;  \
    } else {  \
      v.axis = 0;  \
    }  \
  } while (0)

  fix_coordinate_(x);
  fix_coordinate_(y);

# undef fix_coordinate_

  *pos = *areapos;
  vec2_addeq(&pos->fract, &v);
  locommon_position_reduce(pos);
}

const char* loplayer_camera_state_stringify(loplayer_camera_state_t state) {
# define each_(NAME, name) do {  \
    if (state == LOPLAYER_CAMERA_STATE_##NAME) return #name;  \
  } while (0)

  LOPLAYER_CAMERA_STATE_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool loplayer_camera_state_unstringify(
    loplayer_camera_state_t* state, const char* str, size_t len) {
  assert(state != NULL);

# define each_(NAME, name) do {  \
    if (strncmp(str, #name, len) == 0 && #name[len] == 0) {  \
      *state = LOPLAYER_CAMERA_STATE_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOPLAYER_CAMERA_STATE_EACH_(each_);
  return false;

# undef each_
}

void loplayer_camera_initialize(
    loplayer_camera_t*       camera,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_event_t*  event,
    const loplayer_status_t* status,
    const loplayer_entity_t* entity,
    const mat4_t*            proj) {
  assert(camera  != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(event   != NULL);
  assert(status  != NULL);
  assert(entity  != NULL);
  assert(mat4_valid(proj));

  mat4_t inv_proj;
  mat4_inv(&inv_proj, proj);

  static const vec4_t chunk = vec4(1, 1, 0, 0);
  vec4_t chunksz;
  mat4_mul_vec4(&chunksz, &inv_proj, &chunk);

  *camera = (typeof(*camera)) {
    .shaders = shaders,
    .ticker  = ticker,
    .event   = event,
    .status  = status,
    .entity  = entity,

    .display_chunksz = chunksz.xy,

    .matrix = mat4_scale(1, 1, 1),
    .scale  = 1.0f,

    .cinesco = {
      .color = vec4(0, 0, 0, 1),
    },

    .state      = LOPLAYER_CAMERA_STATE_DEFAULT,
    .brightness = 1,
  };
}

void loplayer_camera_deinitialize(loplayer_camera_t* camera) {
  assert(camera != NULL);

}

void loplayer_camera_update(loplayer_camera_t* camera) {
  assert(camera != NULL);

  const float              d    = camera->ticker->delta_f;
  const loplayer_status_t* stat = camera->status;

  locommon_position_t target = camera->entity->super.super.pos;

  /* ---- movement ---- */
  const loplayer_event_param_t* e = loplayer_event_get_param(camera->event);
  if (e != NULL && e->area_size.x > 0 && e->area_size.y > 0) {
    loplayer_camera_bind_position_in_area_(
        camera, &target, &e->area_pos, &e->area_size);
  }

  vec2_t dist;
  locommon_position_sub(&dist, &target, &camera->pos);
  if (vec2_pow_length(&dist) > 2) camera->pos = target;

  locommon_easing_smooth_position(&camera->pos, &target, d*10);

# define ease_float_(name, ed, speed)  \
      locommon_easing_smooth_float(&camera->name, ed, d*(speed))

  /* ---- cinema scope ---- */
  ease_float_(cinesco.size, !!(e != NULL && e->cinescope)*.3f, 2);

  /* ---- damage effect ---- */
  const bool damaged =
      stat->last_damage_time > 0 &&
      stat->last_damage_time+500 > camera->ticker->time;
  ease_float_(pe.raster, !!damaged*.5f, damaged? 5: 3);

  /* ---- amnesia effect ---- */
  const uint64_t amnesia_st  = stat->recipient.effects.amnesia.begin;
  const uint64_t amnesia_dur = stat->recipient.effects.amnesia.duration;
  ease_float_(
      pe.amnesia_displacement,
      !!(amnesia_st+amnesia_dur > camera->ticker->time), 5);

  /* ---- dying effect ---- */
  const float dying = stat->dying_effect;
  camera->pixsort = dying < .1f? dying/.1f: powf(1-(dying-.1f), 1.5f);
  if (camera->pixsort < 0) camera->pixsort = 0;

  /* ---- switch by current state ---- */
  switch (camera->state) {
  case LOPLAYER_CAMERA_STATE_DEFAULT:
    ease_float_(scale,                  1.0f, 10);
    ease_float_(pe.whole_blur,          0.0f, 1);
    ease_float_(pe.radial_displacement, 0.0f, 5);
    ease_float_(pe.radial_fade,         0.5f, 3);
    break;
  case LOPLAYER_CAMERA_STATE_COMBAT:
    ease_float_(scale,                  1.5f, 8);
    ease_float_(pe.whole_blur,          0.0f, 1);
    ease_float_(pe.radial_displacement, 0.6f, 3);
    ease_float_(pe.radial_fade,         0.8f, 1);
    break;
  case LOPLAYER_CAMERA_STATE_DEAD:
    ease_float_(scale,                  2.0f, 1);
    ease_float_(pe.whole_blur,          1.0f, 1);
    ease_float_(pe.radial_displacement, 0.3f, .7f);
    ease_float_(pe.radial_fade,         1.5f, .7f);
    break;
  case LOPLAYER_CAMERA_STATE_MENU:
    ease_float_(scale,                  1.0f, 10);
    ease_float_(pe.whole_blur,          0.9f, 1);
    ease_float_(pe.radial_displacement, 0.0f, 10);
    ease_float_(pe.radial_fade,         0.6f, 1);
    break;
  }

# undef ease_float_

  /* ---- fixed params ---- */
  camera->pe.brightness = camera->brightness;

  /* ---- matrix ---- */
  camera->matrix = mat4_scale(camera->scale, camera->scale, 1);
}

void loplayer_camera_draw(const loplayer_camera_t* camera) {
  assert(camera != NULL);

  loshader_pixsort_drawer_set_intensity(
      camera->shaders->drawer.pixsort, camera->pixsort);

  loshader_posteffect_drawer_set_param(
      camera->shaders->drawer.posteffect, &camera->pe);

  loshader_cinescope_drawer_set_param(
      camera->shaders->drawer.cinescope, &camera->cinesco);
}

void loplayer_camera_pack(
    const loplayer_camera_t* camera, msgpack_packer* packer) {
  assert(camera != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "state");
  mpkutil_pack_str(packer, loplayer_camera_state_stringify(camera->state));

  mpkutil_pack_str(packer, "pos");
  locommon_position_pack(&camera->pos, packer);
}

bool loplayer_camera_unpack(
    loplayer_camera_t* camera, const msgpack_object* obj) {
  assert(camera != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root  = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)
  const char* v;
  size_t len;
  if (!mpkutil_get_str(item_("state"), &v, &len) ||
      !loplayer_camera_state_unstringify(&camera->state, v, len)) {
    return false;
  }
  if (!locommon_position_unpack(&camera->pos, item_("pos"))) {
    return false;
  }
# undef item_
  return true;
}
