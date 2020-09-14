#include "./context.h"

#include <assert.h>
#include <stddef.h>

#include <ft2build.h>
#include FT_FREETYPE_H

void glyphas_context_initialize(glyphas_context_t* ctx) {
  assert(ctx != NULL);

  *ctx = (typeof(*ctx)) {0};

  FT_Init_FreeType(&ctx->ft);
}

void glyphas_context_deinitialize(glyphas_context_t* ctx) {
  assert(ctx != NULL);

  FT_Done_FreeType(ctx->ft);
}
