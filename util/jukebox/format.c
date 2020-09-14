#include "./format.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool jukebox_format_valid(const jukebox_format_t* format) {
  return format != NULL && format->sample_rate > 0 && format->channels > 0;
}
