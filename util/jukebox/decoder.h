#pragma once

#include <stddef.h>

#include "util/math/rational.h"

#include "./format.h"

struct jukebox_decoder_t;
typedef struct jukebox_decoder_t jukebox_decoder_t;

jukebox_decoder_t*  /* OWNERSHIP */
jukebox_decoder_new_from_file(
    const jukebox_format_t* format,
    const char*             path
);

jukebox_decoder_t*  /* OWNERSHIP */
jukebox_decoder_new_from_memory_mp3(
    const jukebox_format_t* format,
    const void*             buf,
    size_t                  len
);

void
jukebox_decoder_delete(
    jukebox_decoder_t* decoder  /* OWNERSHIP */
);

void
jukebox_decoder_play(
    jukebox_decoder_t* decoder,
    const rational_t*  st,
    bool               loop
);

void
jukebox_decoder_resume(
    jukebox_decoder_t* decoder,
    bool               loop
);

void
jukebox_decoder_stop_after(
    jukebox_decoder_t* decoder,
    const rational_t*  dur
);

void
jukebox_decoder_set_loop(
    jukebox_decoder_t* decoder,
    const rational_t*  start,  /* NULLABLE */
    const rational_t*  end     /* NULLABLE */
);

void
jukebox_decoder_unset_loop(
    jukebox_decoder_t* decoder
);

void
jukebox_decoder_get_seek_position(
    const jukebox_decoder_t* decoder,
    rational_t*              time
);

void
jukebox_decoder_get_duration(
    const jukebox_decoder_t* decoder,
    rational_t*              time
);
