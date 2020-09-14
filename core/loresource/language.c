#include "./language.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define LORESOURCE_LANGUAGE_EACH_(PROC) do {  \
  PROC(JP, jp);  \
} while (0)

const char* loresource_language_stringify(loresource_language_t lang) {
# define each_(NAME, name)  \
      if (lang == LORESOURCE_LANGUAGE_##NAME) return #name;

  LORESOURCE_LANGUAGE_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool loresource_language_unstringify(
    loresource_language_t* lang, const char* str, size_t len) {
  assert(lang != NULL);
  assert(str != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(str, #name, len) == 0 && #name[len] == 0) {  \
        *lang = LORESOURCE_LANGUAGE_##NAME;  \
        return true;  \
    }  \
  } while (0)

  LORESOURCE_LANGUAGE_EACH_(each_);
  return false;

# undef each_
}
