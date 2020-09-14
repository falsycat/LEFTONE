#pragma once

#include <stdbool.h>

#include "util/math/vector.h"

bool
coly2d_hittest_point_and_rect(
    const vec2_t* pos1,
    const vec2_t* pos2,
    const vec2_t* size2
);

bool
coly2d_hittest_point_and_triangle(
    const vec2_t* pos1,
    const vec2_t* pos2_a,
    const vec2_t* pos2_b,
    const vec2_t* pos2_c
);

bool
coly2d_hittest_lineseg_and_lineseg(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2_st,
    const vec2_t* pos2_ed
);

bool
coly2d_hittest_lineseg_and_rect(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2,
    const vec2_t* size2
);

bool
coly2d_hittest_lineseg_and_ellipse(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2,
    const vec2_t* size2
);

bool
coly2d_hittest_lineseg_and_triangle(
    const vec2_t* pos1_st,
    const vec2_t* pos1_ed,
    const vec2_t* pos2_a,
    const vec2_t* pos2_b,
    const vec2_t* pos2_c
);
