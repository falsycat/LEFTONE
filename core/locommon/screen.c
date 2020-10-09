#include "./screen.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/math/matrix.h"
#include "util/math/vector.h"

#define CM_PER_INCH_ 2.54f

#define CHUNK_INCH_ 16
#define MAX_SCALE_  (1/.5f)

bool locommon_screen_valid(const locommon_screen_t* screen) {
  return
      screen != NULL &&
      vec2_valid(&screen->resolution) &&
      vec2_valid(&screen->dpi) &&
      screen->resolution.x > 0 &&
      screen->resolution.y > 0 &&
      screen->dpi.x        > 0 &&
      screen->dpi.y        > 0;
}

void locommon_screen_calc_pixels_from_cm(
    const locommon_screen_t* screen, vec2_t* pixels, const vec2_t* cm) {
  assert(locommon_screen_valid(screen));
  assert(pixels != NULL);
  assert(vec2_valid(cm));

  locommon_screen_calc_pixels_from_inch(
      screen, pixels, &vec2(cm->x/CM_PER_INCH_, cm->y/CM_PER_INCH_));
}

void locommon_screen_calc_pixels_from_inch(
    const locommon_screen_t* screen, vec2_t* pixels, const vec2_t* inch) {
  assert(locommon_screen_valid(screen));
  assert(pixels != NULL);
  assert(vec2_valid(inch));

  *pixels = *inch;

  pixels->x *= screen->dpi.x;
  pixels->y *= screen->dpi.y;
}

void locommon_screen_calc_winpos_from_cm(
    const locommon_screen_t* screen, vec2_t* winpos, const vec2_t* cm) {
  assert(locommon_screen_valid(screen));
  assert(winpos != NULL);
  assert(vec2_valid(cm));

  locommon_screen_calc_winpos_from_inch(
      screen, winpos, &vec2(cm->x/CM_PER_INCH_, cm->y/CM_PER_INCH_));
}

void locommon_screen_calc_winpos_from_inch(
    const locommon_screen_t* screen, vec2_t* winpos, const vec2_t* inch) {
  assert(locommon_screen_valid(screen));
  assert(winpos != NULL);
  assert(vec2_valid(inch));

  locommon_screen_calc_pixels_from_inch(screen, winpos, inch);
  winpos->x /= screen->resolution.x / 2;
  winpos->y /= screen->resolution.y / 2;
}

void locommon_screen_calc_winpos_from_pixels(
    const locommon_screen_t* screen, vec2_t* winpos, const vec2_t* pixels) {
  assert(locommon_screen_valid(screen));
  assert(winpos != NULL);
  assert(vec2_valid(pixels));

  *winpos = *pixels;
  winpos->x /= screen->resolution.x / 2;
  winpos->y /= screen->resolution.y / 2;
}

void locommon_screen_build_projection_matrix(
    const locommon_screen_t* screen, mat4_t* proj) {
  assert(screen != NULL);
  assert(proj   != NULL);

  vec2_t scale;
  locommon_screen_calc_winpos_from_inch(
      screen, &scale, &vec2(CHUNK_INCH_, CHUNK_INCH_));

  if (scale.x > MAX_SCALE_) {
    scale.y *= MAX_SCALE_/scale.x;
    scale.x  = MAX_SCALE_;
  }
  *proj = mat4_scale(scale.x, scale.y, 1);
}
