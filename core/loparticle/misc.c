#include "./misc.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

const char* loparticle_type_stringify(loparticle_type_t type) {
# define each_(NAME, name) do {  \
    if (type == LOPARTICLE_TYPE_##NAME) return #name;  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);

# undef each_

  assert(false);
  return NULL;
}

bool loparticle_type_unstringify(
    loparticle_type_t* type, const char* v, size_t len) {
  assert(type != NULL);
  assert(v != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(v, #name, len) == 0 && #name[len] == 0) {  \
      *type = LOPARTICLE_TYPE_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOPARTICLE_TYPE_EACH_(each_);
  return false;

# undef each_
}
