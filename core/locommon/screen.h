#pragma once

#include <stdbool.h>

#include "util/math/matrix.h"
#include "util/math/vector.h"

typedef struct {
  vec2_t resolution;
  vec2_t dpi;
} locommon_screen_t;

bool
locommon_screen_valid(
    const locommon_screen_t* screen
);

void
locommon_screen_calc_pixels_from_cm(
    const locommon_screen_t* screen,
    vec2_t*                  pixels,
    const vec2_t*            cm
);

void
locommon_screen_calc_pixels_from_inch(
    const locommon_screen_t* screen,
    vec2_t*                  pixels,
    const vec2_t*            inch
);

void
locommon_screen_calc_winpos_from_cm(
    const locommon_screen_t* screen,
    vec2_t*                  winpos,
    const vec2_t*            cm
);

void
locommon_screen_calc_winpos_from_inch(
    const locommon_screen_t* screen,
    vec2_t*                  winpos,
    const vec2_t*            inch
);

void
locommon_screen_calc_winpos_from_pixels(
    const locommon_screen_t* screen,
    vec2_t*                  winpos,
    const vec2_t*            pixels
);

void
locommon_screen_build_projection_matrix(
    const locommon_screen_t* screen,
    mat4_t*                  proj
);
