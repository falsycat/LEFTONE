#define SDL_MAIN_HANDLED
#define NO_STDIO_REDIRECT

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include <GL/glew.h>
#include <SDL2/SDL.h>

#include "core/loscene/context.h"

#include "./args.h"
#include "./event.h"

static const app_args_t app_default_args_ = {
  .scene = {
    .width    = 960,
    .height   = 540,
    .dpi      = vec2(96, 96),

    .max_msaa   = 8,
    .brightness = 1000,
  },
  .max_fps = 60,
};

typedef struct {
  SDL_Window*   win;
  SDL_GLContext gl;
} libs_t;

static void app_initialize_libraries_(libs_t* libs, app_args_t* args) {
  assert(libs != NULL);
  assert(args != NULL);

  SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
  if (SDL_Init(SDL_INIT_VIDEO)) {
    fprintf(stderr, "failed to initialize SDL: %s\n", SDL_GetError());
    abort();
  }

  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(
      SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  uint32_t win_flags =
      SDL_WINDOW_ALLOW_HIGHDPI |
      SDL_WINDOW_OPENGL |
      SDL_WINDOW_SHOWN;
  if (args->force_window) {
  } else if (args->force_desktop_fullscreen) {
    win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  } else if (args->force_fullscreen) {
    win_flags |= SDL_WINDOW_FULLSCREEN;
  } else {
# ifdef NDEBUG
    win_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
# endif
  }

  libs->win = SDL_CreateWindow(
      "LEFTONE",               /* title */
      SDL_WINDOWPOS_CENTERED,  /* X position */
      SDL_WINDOWPOS_CENTERED,  /* Y position */
      args->scene.width,
      args->scene.height,
      win_flags);
  if (libs->win == NULL) {
    fprintf(stderr, "failed to create window: %s\n", SDL_GetError());
    abort();
  }

  int w, h;
  SDL_GetWindowSize(libs->win, &w, &h);
  args->scene.width  = w;
  args->scene.height = h;

  libs->gl = SDL_GL_CreateContext(libs->win);

  glewExperimental = 1;
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "failed to init GLEW\n");
    abort();
  }

  glViewport(0, 0, args->scene.width, args->scene.height);
}
static void app_deinitialize_libraries_(libs_t* libs) {
  assert(libs != NULL);

  SDL_GL_DeleteContext(libs->gl);
  SDL_DestroyWindow(libs->win);
  SDL_Quit();
}

static void app_get_dpi_(libs_t* libs, app_args_t* args) {
  assert(libs != NULL);
  assert(args != NULL);

  if (args->override_dpi) return;

  const int32_t disp = SDL_GetWindowDisplayIndex(libs->win);

  float x, y;
  if (SDL_GetDisplayDPI(disp, NULL, &x, &y) == 0) {
    args->scene.dpi = vec2(x, y);
  } else {
    fprintf(stderr, "failed to get display DPI: %s\n", SDL_GetError());
    fprintf(stderr, "Anti-aliasing may not work properly.\n");
  }
}

int main(int argc, char** argv) {
  (void) argc, (void) argv;

  app_args_t args = app_default_args_;
  app_args_parse(&args, argc, argv);

  libs_t libs;
  app_initialize_libraries_(&libs, &args);

  app_get_dpi_(&libs, &args);

  loscene_context_t* ctx = loscene_context_new(&args.scene);

  locommon_input_t input = {
    .resolution = vec2(args.scene.width, args.scene.height),
    .dpi        = args.scene.dpi,
    .cursor     = vec2(0, 0),
  };

  glClearColor(0, 0, 0, 0);

  const uint64_t min_frame_time = 1000 / args.max_fps;
  for (;;) {
    const uint64_t base_time = SDL_GetTicks();

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (!app_event_handle(&input, &e)) goto EXIT;
    }

    if (!loscene_context_update(ctx, &input, base_time)) goto EXIT;

    glClear(GL_COLOR_BUFFER_BIT);
    loscene_context_draw(ctx);
    SDL_GL_SwapWindow(libs.win);

#   ifndef NDEBUG
      /* for debugging in MSYS */
      fflush(stdout);
      fflush(stderr);
#   endif

    const uint64_t elapsed = SDL_GetTicks() - base_time;
    if (elapsed < min_frame_time) {
      SDL_Delay(min_frame_time - elapsed);
    }
  }

EXIT:
  loscene_context_delete(ctx);
  app_deinitialize_libraries_(&libs);
  return 0;
}
