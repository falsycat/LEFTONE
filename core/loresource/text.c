#include "./text.h"

#include <assert.h>

#include "util/dictres/dictres.h"

#include "./language.h"

/* text resource */
#include "./text/jp.h"

#define LORESOURCE_TEXT_SWITCH_BY_LANG(lang, each) do {  \
  switch (lang) {  \
  case LORESOURCE_LANGUAGE_JP: each(loresource_text_jp_); break;  \
  default: assert(false);  \
  }  \
} while (0)

void loresource_text_optimize(loresource_language_t lang) {
# define each_(i) dictres_optimize(i)
  LORESOURCE_TEXT_SWITCH_BY_LANG(lang, each_);
# undef each_
}

const char* loresource_text_get(loresource_language_t lang, const char* key) {
# define each_(i) return dictres_find(i, key)
  LORESOURCE_TEXT_SWITCH_BY_LANG(lang, each_);

  assert(false);
  return NULL;
# undef each_
}
