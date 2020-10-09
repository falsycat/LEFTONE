#include "./drawer.h"

#include <assert.h>
#include <stddef.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/texture.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./block.h"

/* resources */
#include "util/glyphas/anysrc/drawer.vshader.h"
#include "util/glyphas/anysrc/drawer.fshader.h"

#define UNIFORM_TEX_ 0

#define VSHADER_IN_POS_     0
#define VSHADER_IN_SIZE_    1
#define VSHADER_IN_UV_POS_  2
#define VSHADER_IN_UV_SIZE_ 3
#define VSHADER_IN_COLOR_   4

#pragma pack(push, 1)
typedef struct {
  vec2_t pos;
  vec2_t size;
  vec2_t uv_pos;
  vec2_t uv_size;
  vec4_t color;
} glyphas_drawer_instance_t;
#pragma pack(pop)

gleasy_program_t glyphas_drawer_create_default_program(void) {
  return gleasy_program_new("", 0,
      glyphas_drawer_vshader_, sizeof(glyphas_drawer_vshader_),
      glyphas_drawer_fshader_, sizeof(glyphas_drawer_fshader_));
}

void glyphas_drawer_initialize(
    glyphas_drawer_t* drawer, gleasy_texture_2d_t tex) {
  assert(drawer != NULL);

  *drawer = (typeof(*drawer)) {
    .tex = tex,
  };

  glCreateVertexArrays(1, &drawer->vao);
  glGenBuffers(1, &drawer->instances);

  glBindVertexArray(drawer->vao);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);

# define enable_(index, var, dim, type) do {  \
    glEnableVertexAttribArray(index);  \
    glVertexAttribPointer(  \
        index, dim, type, GL_FALSE,  \
        sizeof(glyphas_drawer_instance_t),  \
        NULL + offsetof(glyphas_drawer_instance_t, var));  \
    glVertexAttribDivisor(index, 1);  \
  } while (0)

  enable_(VSHADER_IN_POS_,     pos,     2, GL_FLOAT);
  enable_(VSHADER_IN_SIZE_,    size,    2, GL_FLOAT);
  enable_(VSHADER_IN_UV_POS_,  uv_pos,  2, GL_FLOAT);
  enable_(VSHADER_IN_UV_SIZE_, uv_size, 2, GL_FLOAT);
  enable_(VSHADER_IN_COLOR_,   color,   4, GL_FLOAT);

# undef enable_attrib_
}

void glyphas_drawer_deinitialize(glyphas_drawer_t* drawer) {
  assert(drawer != NULL);

  glDeleteBuffers(1, &drawer->instances);
  glDeleteVertexArrays(1, &drawer->vao);
}

void glyphas_drawer_clear(glyphas_drawer_t* drawer, size_t reserve) {
  assert(drawer != NULL);
  assert(reserve > 0);

  drawer->instances_length = 0;
  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve*sizeof(glyphas_drawer_instance_t), NULL, GL_DYNAMIC_DRAW);
    drawer->instances_reserved = reserve;
  }
}

void glyphas_drawer_add_block(
    glyphas_drawer_t* drawer, const glyphas_block_t* block) {
  assert(drawer != NULL);
  assert(block  != NULL);

  size_t len;
  const glyphas_block_item_t* items = glyphas_block_get_items(block, &len);

  for (size_t i = 0; i < len; ++i) {
    glyphas_drawer_add_block_item(drawer, &items[i]);
  }
}

void glyphas_drawer_add_block_item(
    glyphas_drawer_t* drawer, const glyphas_block_item_t* item) {
  assert(drawer != NULL);
  assert(item   != NULL);

  const glyphas_drawer_instance_t insta = {
    .pos     = item->pos,
    .size    = item->size,
    .uv_pos  = vec2(item->uv.left, item->uv.top),
    .uv_size = vec2(item->uv.right-item->uv.left, item->uv.top-item->uv.bottom),
    .color   = item->color,
  };

  const size_t offset = drawer->instances_length * sizeof(insta);
  glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
  glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(insta), &insta);

  ++drawer->instances_length;
}

void glyphas_drawer_draw(const glyphas_drawer_t* drawer) {
  assert(drawer != NULL);

  if (drawer->instances_length == 0) return;

  glBindVertexArray(drawer->vao);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, drawer->tex);
  glUniform1i(UNIFORM_TEX_, 0);

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, drawer->instances_length);
}
