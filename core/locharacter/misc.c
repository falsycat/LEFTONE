#include "./misc.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

const char* locharacter_type_stringify(locharacter_type_t type) {
# define each_(NAME, name) do {  \
    if (type == LOCHARACTER_TYPE_##NAME) return #name;  \
  } while(0)

  LOCHARACTER_TYPE_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool locharacter_type_unstringify(
    locharacter_type_t* type, const char* v, size_t len) {
  assert(type != NULL);
  assert(v != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(v, #name, len) == 0 && #name[len] == 0) {  \
      *type = LOCHARACTER_TYPE_##NAME;  \
      return true;  \
    }  \
  } while(0)

  LOCHARACTER_TYPE_EACH_(each_);
  return false;

# undef each_
}

const char* locharacter_state_stringify(locharacter_state_t state) {
# define each_(NAME, name) do {  \
    if (state == LOCHARACTER_STATE_##NAME) return #name;  \
  } while(0)

  LOCHARACTER_STATE_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool locharacter_state_unstringify(
    locharacter_state_t* state, const char* v, size_t len) {
  assert(state != NULL);
  assert(v != NULL || len == 0);

# define each_(NAME, name) do {  \
    if (strncmp(v, #name, len) == 0 && #name[len] == 0) {  \
      *state = LOCHARACTER_STATE_##NAME;  \
      return true;  \
    }  \
  } while(0)

  LOCHARACTER_STATE_EACH_(each_);
  return false;

# undef each_
}
