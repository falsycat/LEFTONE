#pragma once

#include <stdatomic.h>
#include <stddef.h>

#include "util/math/rational.h"

#include "./format.h"

struct jukebox_sound_buffer_t;
typedef struct jukebox_sound_buffer_t jukebox_sound_buffer_t;

struct jukebox_sound_t;
typedef struct jukebox_sound_t jukebox_sound_t;

jukebox_sound_buffer_t*
jukebox_sound_buffer_new_from_file(
    const jukebox_format_t* format,
    const char*             path
);

jukebox_sound_buffer_t*
jukebox_sound_buffer_new_from_memory_mp3(
    const jukebox_format_t* format,
    const void*             buf,
    size_t                  len
);

void
jukebox_sound_buffer_delete(
    jukebox_sound_buffer_t* buf
);

jukebox_sound_t*
jukebox_sound_new(
    const jukebox_sound_buffer_t* buf,
    size_t                        max_concurrent
);

void
jukebox_sound_delete(
    jukebox_sound_t* sound
);

bool
jukebox_sound_play(
    jukebox_sound_t* sound
);

void
jukebox_sound_stop_all(
    jukebox_sound_t* sound
);
