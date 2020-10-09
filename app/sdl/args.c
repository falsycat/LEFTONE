#include "./args.h"

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util/parsarg/parsarg.h"

#include "core/loscene/param.h"

void app_args_parse(app_args_t* args, int argc, char** argv) {
  assert(args != NULL);
  assert(argc > 0);

  parsarg_t pa;
  parsarg_initialize(&pa, argc-1, argv+1);

  while (!parsarg_finished(&pa)) {
    size_t nlen;
    char* n = parsarg_pop_name(&pa, &nlen);

    char* v;
    parsarg_pop_value(&pa, &v);

    if (n == NULL && v == NULL) continue;

    bool ok = false;

#   define bool_(name, var) do {  \
      if (strncmp(name, n, nlen) == 0 && name[nlen] == 0) {  \
        if (v == NULL) {  \
          var = true;  \
          ok  = true;  \
        } else {  \
          fprintf(stderr, "option '"name"' cannot take any values");  \
          abort();  \
        }  \
      }  \
    } while (0)

#   define int_(name, var, min, max) do {  \
      if (strncmp(name, n, nlen) == 0 && name[nlen] == 0) {  \
        char* end;  \
        const intmax_t i = strtoimax(v, &end, 0);  \
        if (*end == 0 && min <= i && i < max) {  \
          var = i;  \
          ok  = true;  \
        } else {  \
          fprintf(stderr,  \
              "option '"name"' requires "  \
              "an integer value (%"PRIdMAX"~%"PRIdMAX")\n",  \
              (intmax_t) min, (intmax_t) max);  \
          abort();  \
        }  \
        continue;  \
      }  \
    } while (0)

    /* ---- scene parameters ---- */
    int_("width",   args->scene.width,    640, INT32_MAX);
    int_("height",  args->scene.height,   360, INT32_MAX);
    int_("dpi-x",   args->scene.dpi.x,      1, INT32_MAX);
    int_("dpi-y",   args->scene.dpi.y,      1, INT32_MAX);

    int_("max-msaa",   args->scene.max_msaa,   1, INT32_MAX);
    int_("brightness", args->scene.brightness, 0, 2000);

    bool_("disable-heavy-backwall",
        args->scene.environment.disable_heavy_backwall);
    bool_("disable-heavy-fog",
        args->scene.environment.disable_heavy_fog);

    bool_("skip-title", args->scene.skip_title);

    /* ---- app parameters ---- */
    int_("max-fps", args->max_fps, 1, INT32_MAX);

    bool_("override-dpi", args->override_dpi);

    bool_("force-window",             args->force_window);
    bool_("force-desktop-fullscreen", args->force_desktop_fullscreen);
    bool_("force-fullscreen",         args->force_fullscreen);

#   undef int_
#   undef bool_

    if (!ok) {
      if (n == NULL) {
        fprintf(stderr, "missing option name for the value '%s'\n", v);
      } else {
        fprintf(stderr, "unknown option '%.*s'\n", (int) nlen, n);
      }
      abort();
    }
  }

  parsarg_deinitialize(&pa);
}
