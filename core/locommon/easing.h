#pragma once

#include "./position.h"

void
locommon_easing_linear_float(
    float* v,
    float  ed,
    float  delta
);

void
locommon_easing_smooth_float(
    float* v,
    float  ed,
    float  delta
);

void
locommon_easing_smooth_position(
    locommon_position_t*       pos,
    const locommon_position_t* ed,
    float                      delta
);
