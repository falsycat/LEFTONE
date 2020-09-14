#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
  int32_t sample_rate;
  int32_t channels;
} jukebox_format_t;

bool
jukebox_format_valid(
    const jukebox_format_t* format
);
