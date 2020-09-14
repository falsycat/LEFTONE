#pragma once

#include <stdbool.h>

#include "core/loscene/param.h"

typedef struct {
  loscene_param_t scene;

  int32_t max_fps;

  bool override_dpi;

  bool force_window;
  bool force_desktop_fullscreen;
  bool force_fullscreen;
} app_args_t;

void
app_args_parse(
    app_args_t* args,
    int         argc,
    char**      argv
);
