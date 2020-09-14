#include "./drawer.h"

#include <assert.h>
#include <stddef.h>

#include <GL/glew.h>

#include "util/gleasy/buffer.h"
#include "util/gleasy/texture.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"
#include "util/memory/memory.h"

#include "./block.h"

/* resources */
#include "anysrc/drawer.vshader.h"
#include "anysrc/drawer.fshader.h"

#define GLYPHAS_DRAWER_UNIFORM_TEX 0

#define GLYPHAS_DRAWER_VSHADER_IN_POS     0
#define GLYPHAS_DRAWER_VSHADER_IN_SIZE    1
#define GLYPHAS_DRAWER_VSHADER_IN_UV_POS  2
#define GLYPHAS_DRAWER_VSHADER_IN_UV_SIZE 3
#define GLYPHAS_DRAWER_VSHADER_IN_COLOR   4

struct glyphas_drawer_t {
  gleasy_texture_2d_t tex;

  GLuint vao;

  gleasy_buffer_array_t instances;
  size_t                instances_reserved;
  size_t                instances_length;
};

#pragma pack(push, 1)
typedef struct {
  vec2_t pos;
  vec2_t size;
  vec2_t uv_pos;
  vec2_t uv_size;
  vec4_t color;
} glyphas_drawer_instance_t;
#pragma pack(pop)

static void glyphas_drawer_setup_vao_(
    gleasy_buffer_array_t instances) {
  assert(instances != 0);

  glBindBuffer(GL_ARRAY_BUFFER, instances);

# define enable_attrib_(NAME, name, dim, type) do {  \
    glEnableVertexAttribArray(GLYPHAS_DRAWER_VSHADER_IN_##NAME);  \
    glVertexAttribPointer(  \
        GLYPHAS_DRAWER_VSHADER_IN_##NAME, dim, type, GL_FALSE,  \
        sizeof(glyphas_drawer_instance_t),  \
        NULL + offsetof(glyphas_drawer_instance_t, name));  \
    glVertexAttribDivisor(GLYPHAS_DRAWER_VSHADER_IN_##NAME, 1);  \
  } while (0)

  enable_attrib_(POS,     pos,     2, GL_FLOAT);
  enable_attrib_(SIZE,    size,    2, GL_FLOAT);
  enable_attrib_(UV_POS,  uv_pos,  2, GL_FLOAT);
  enable_attrib_(UV_SIZE, uv_size, 2, GL_FLOAT);
  enable_attrib_(COLOR,   color,   4, GL_FLOAT);

# undef enable_attrib_
}

gleasy_program_t glyphas_drawer_create_default_program(void) {
  return gleasy_program_new("", 0,
      glyphas_drawer_vshader_, sizeof(glyphas_drawer_vshader_),
      glyphas_drawer_fshader_, sizeof(glyphas_drawer_fshader_));
}

glyphas_drawer_t* glyphas_drawer_new(void) {
  glyphas_drawer_t* drawer = memory_new(sizeof(*drawer));
  *drawer = (typeof(*drawer)) {0};

  glCreateVertexArrays(1, &drawer->vao);
  glBindVertexArray(drawer->vao);

  glGenBuffers(1, &drawer->instances);
  glyphas_drawer_setup_vao_(drawer->instances);

  return drawer;
}

void glyphas_drawer_delete(glyphas_drawer_t* drawer) {
  if (drawer == NULL) return;

  glDeleteBuffers(1, &drawer->instances);

  glDeleteVertexArrays(1, &drawer->vao);

  memory_delete(drawer);
}

void glyphas_drawer_clear(
    glyphas_drawer_t* drawer, gleasy_texture_2d_t tex, size_t reserve) {
  assert(drawer != NULL);
  assert(tex    != 0);
  assert(reserve > 0);

  drawer->tex = tex;

  drawer->instances_length = 0;
  if (drawer->instances_reserved < reserve) {
    glBindBuffer(GL_ARRAY_BUFFER, drawer->instances);
    glBufferData(GL_ARRAY_BUFFER,
        reserve * sizeof(glyphas_drawer_instance_t),
        NULL, GL_DYNAMIC_DRAW);
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
  glUniform1i(GLYPHAS_DRAWER_UNIFORM_TEX, 0);

  glDrawArraysInstanced(GL_TRIANGLES, 0, 6, drawer->instances_length);
}
