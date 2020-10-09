#include "./face.h"

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <ft2build.h>
#include FT_ERRORS_H
#include FT_FREETYPE_H

#include "./context.h"

static const char* glyphas_face_get_ft_error_str_(FT_Error err) {
  /* what a fucking trick lol.
   * https://stackoverflow.com/questions/61641364/gcc-cant-find-ft-error-string-when-trying-to-compile */
# undef FTERRORS_H_
# define FT_ERRORDEF(code, val, str) case code: return str;
# define FT_ERROR_START_LIST switch(err) {
# define FT_ERROR_END_LIST default: return "unknown"; }
# include FT_ERRORS_H
}

void glyphas_face_initialize_from_file(
    glyphas_face_t*          face,
    const glyphas_context_t* ctx,
    const char*              path,
    size_t                   index) {
  assert(face != NULL);
  assert(ctx  != NULL);

  *face = (typeof(*face)) {0};

  const FT_Error err = FT_New_Face(ctx->ft, path, index, &face->ft);
  if (err != FT_Err_Ok) {
    fprintf(stderr, "failed to load font file '%s': %s\n",
        path, glyphas_face_get_ft_error_str_(err));
    abort();
  }
}

void glyphas_face_initialize_from_buffer(
    glyphas_face_t*          face,
    const glyphas_context_t* ctx,
    const void*              data,
    size_t                   length,
    size_t                   index) {
  assert(face != NULL);
  assert(ctx  != NULL);

  *face = (typeof(*face)) {0};

  const FT_Error err =
      FT_New_Memory_Face(ctx->ft, data, length, index, &face->ft);
  if (err != FT_Err_Ok) {
    fprintf(stderr, "failed to load font on memory: %s\n",
        glyphas_face_get_ft_error_str_(err));
    abort();
  }
}

void glyphas_face_deinitialize(glyphas_face_t* face) {
  assert(face != NULL);

  FT_Done_Face(face->ft);
}

bool glyphas_face_set_pixel_size(
    glyphas_face_t* face, int32_t width, int32_t height) {
  assert(face != NULL);
  assert(width  > 0);
  assert(height > 0);

  const FT_Error err = FT_Set_Pixel_Sizes(face->ft, width, height);
  return err == FT_Err_Ok;
}

bool glyphas_face_render_glyph(glyphas_face_t* face, uint64_t unicode) {
  assert(face != NULL);

  const FT_UInt index = FT_Get_Char_Index(face->ft, unicode);
  if (index == 0) return false;

  const FT_Error err = FT_Load_Glyph(face->ft, index, FT_LOAD_RENDER);
  if (err != FT_Err_Ok) return false;

  return glyphas_glyph_reset_from_ft_glyph_slot(&face->glyph, face->ft->glyph);
}
