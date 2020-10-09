/* An expression in assert() can be expected to be executed absolutely. */
#undef NDEBUG

#define SDL_MAIN_HANDLED

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "util/conv/charcode.h"
#include "util/gleasy/buffer.h"
#include "util/gleasy/program.h"
#include "util/math/vector.h"

#include "./cache.h"
#include "./context.h"
#include "./drawer.h"
#include "./face.h"
#include "./glyph.h"

typedef struct {
  glyphas_context_t glyphas;
  glyphas_face_t    face;

  gleasy_atlas_t*  atlas;
  glyphas_cache_t* cache;

  gleasy_program_t prog;
  glyphas_drawer_t drawer;
} context_t;

#define SCALE 0.01f

static void align_text_(context_t* ctx, const char* str) {
  assert(ctx != NULL);

  const size_t len = strlen(str);
  glyphas_block_t* block = glyphas_block_new(
      GLYPHAS_ALIGNER_DIRECTION_HORIZONTAL, -20, 400, len);

  static vec4_t color = vec4(0, 0, 0, 1);
  glyphas_block_add_characters(block, ctx->cache, &color, str, len);

  vec2_t offset, size;
  glyphas_block_calculate_geometry(block, &offset, &size);

  offset = vec2(-offset.x, 0);
  glyphas_block_translate(block, &offset);

  size = vec2(1.f/200, 1.f/200);
  glyphas_block_scale(block, &size);

  static const vec2_t origin = vec2(.5f, -.5f);
  glyphas_block_set_origin(block, &origin);

  glyphas_drawer_clear(&ctx->drawer, len);
  glyphas_drawer_add_block(&ctx->drawer, block);
  glyphas_block_delete(block);
}

static void initialize_(context_t* ctx, const char* path, const char* str) {
  assert(ctx != NULL);

  *ctx = (typeof(*ctx)) {0};
  glyphas_context_initialize(&ctx->glyphas);

  glyphas_face_initialize_from_file(&ctx->face, &ctx->glyphas, path, 0);

  ctx->atlas = gleasy_atlas_new(GL_RED, 256, 256, false  /* anti-alias */);
  ctx->cache = glyphas_cache_new(ctx->atlas, &ctx->face, 16, 16);

  ctx->prog = glyphas_drawer_create_default_program();
  glyphas_drawer_initialize(&ctx->drawer, gleasy_atlas_get_texture(ctx->atlas));
  assert(glGetError() == GL_NO_ERROR);

  align_text_(ctx, str);
  assert(glGetError() == GL_NO_ERROR);
}

static void draw_(const context_t* ctx) {
  assert(ctx != NULL);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ctx->prog);
  glyphas_drawer_draw(&ctx->drawer);
}

static void deinitialize_(context_t* ctx) {
  assert(ctx != NULL);

  glyphas_drawer_deinitialize(&ctx->drawer);
  glDeleteProgram(ctx->prog);

  glyphas_cache_delete(ctx->cache);
  gleasy_atlas_delete(ctx->atlas);

  glyphas_face_deinitialize(&ctx->face);
  glyphas_context_deinitialize(&ctx->glyphas);
}

typedef struct {
  SDL_Window*   win;
  SDL_GLContext gl;
} libs_t;

static void initialize_libraries_(libs_t* libs) {
  assert(libs != NULL);

  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  assert(SDL_Init(SDL_INIT_VIDEO) == 0);

  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  static const uint32_t win_flags =
      SDL_WINDOW_ALLOW_HIGHDPI |
      SDL_WINDOW_OPENGL |
      SDL_WINDOW_SHOWN;
  libs->win = SDL_CreateWindow(
      "glyphas-test",               /* title */
      SDL_WINDOWPOS_CENTERED,  /* X position */
      SDL_WINDOWPOS_CENTERED,  /* Y position */
      400,
      400,
      win_flags);
  assert(libs->win != NULL);

  libs->gl = SDL_GL_CreateContext(libs->win);

  glewExperimental = 1;
  assert(glewInit() == GLEW_OK);
}
static void deinitialize_libraries_(libs_t* libs) {
  assert(libs != NULL);

  SDL_GL_DeleteContext(libs->gl);
  SDL_DestroyWindow(libs->win);
  SDL_Quit();
}

int main(int argc, char** argv) {
  assert(argc == 3);

  libs_t libs;
  initialize_libraries_(&libs);

  context_t ctx;
  initialize_(&ctx, argv[1], argv[2]);

  bool alive = true;
  while (alive) {
    SDL_Delay(30);

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) alive = false;
    }
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_(&ctx);
    SDL_GL_SwapWindow(libs.win);
  }

  deinitialize_(&ctx);
  deinitialize_libraries_(&libs);
  return EXIT_SUCCESS;
}
