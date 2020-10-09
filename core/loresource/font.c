#include "./font.h"

#include <assert.h>
#include <stddef.h>

#include "util/glyphas/context.h"
#include "util/glyphas/face.h"

/* resources */
#include "core/loresource/anysrc/font/sans.woff.h"
#include "core/loresource/anysrc/font/serif.woff.h"

void loresource_font_initialize(loresource_font_t* font) {
  assert(font != NULL);

  *font = (typeof(*font)) {0};

  glyphas_context_initialize(&font->glyphas);

# define load_face_(name)  \
      glyphas_face_initialize_from_buffer(  \
          &font->name,  \
          &font->glyphas,  \
          loresource_font_##name##_woff_,  \
          sizeof(loresource_font_##name##_woff_),  \
          0)

  load_face_(sans);
  load_face_(serif);

# undef load_face_
}

void loresource_font_deinitialize(loresource_font_t* font) {
  assert(font != NULL);

  glyphas_face_deinitialize(&font->sans);
  glyphas_face_deinitialize(&font->serif);

  glyphas_context_deinitialize(&font->glyphas);
}
