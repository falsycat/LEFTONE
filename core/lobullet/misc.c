#include "./misc.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

const char* lobullet_type_stringify(lobullet_type_t type) {
# define each_(NAME, name) do {  \
    if (type == LOBULLET_TYPE_##NAME) return #name;  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool lobullet_type_unstringify(
    lobullet_type_t* type, const char* v, size_t len) {
  assert(type != NULL);
  assert(v != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(v, #name, len) == 0 && #name[len] == 0) {  \
      *type = LOBULLET_TYPE_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOBULLET_TYPE_EACH_(each_);
  return false;

# undef each_
}
