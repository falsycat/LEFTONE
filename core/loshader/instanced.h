#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"

#include "./uniblock.h"

typedef struct {
  gleasy_program_t           prog;
  const loshader_uniblock_t* uniblock;

  GLuint vao;

  size_t instance_size;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
} loshader_instanced_drawer_t;

void
loshader_instanced_drawer_initialize(
    loshader_instanced_drawer_t* drawer,
    gleasy_program_t             prog,  /* OWNERSHIP */
    const loshader_uniblock_t*   uniblock,
    size_t                       instance_size
);

void
loshader_instanced_drawer_deinitialize(
    loshader_instanced_drawer_t* drawer
);

void
loshader_instanced_drawer_clear(
    loshader_instanced_drawer_t* drawer,
    size_t                       reserve
);

bool
loshader_instanced_drawer_add_instance(
    loshader_instanced_drawer_t* drawer,
    const void*                  instance
);

void
loshader_instanced_drawer_draw(
    const loshader_instanced_drawer_t* drawer,
    size_t                             primitives
);
