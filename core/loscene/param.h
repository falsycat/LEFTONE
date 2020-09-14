#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "util/math/vector.h"

#include "core/loworld/environment.h"

typedef struct {
  int32_t width;
  int32_t height;
  vec2_t  dpi;

  int32_t max_msaa;
  int32_t brightness;  /* darkest |0 <- 1000 -> 2000| brightest */

  loworld_environment_config_t environment;

  bool skip_title;

  struct {
    bool loworld_poolset_packing;
    bool loplayer_packing;
  } test;
} loscene_param_t;
