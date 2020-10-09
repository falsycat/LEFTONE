#pragma once

#include <stddef.h>
#include <stdint.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef struct {
  gleasy_program_t prog;
  const loshader_uniblock_t* uniblock;

  gleasy_buffer_uniform_t param;
  size_t                  param_size;
} loshader_single_drawer_t;

void
loshader_single_drawer_initialize(
    loshader_single_drawer_t*  drawer,
    gleasy_program_t           prog,  /* OWNERSHIP */
    const loshader_uniblock_t* uniblock,
    size_t                     param_size
);

void
loshader_single_drawer_deinitialize(
    loshader_single_drawer_t* drawer
);

void
loshader_single_drawer_set_param(
    loshader_single_drawer_t* drawer,
    const void*               ptr
);

void
loshader_single_drawer_draw(
    const loshader_single_drawer_t* drawer,
    size_t                          primitives
);

void
loshader_single_drawer_draw_without_use_program(
    const loshader_single_drawer_t* drawer,
    size_t                          primitives
);
