#include "./stance.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/loshader/menu_stance.h"

#include "./recipient.h"

const char* loeffect_stance_stringify(loeffect_stance_id_t id) {
# define each_(NAME, name)  \
      if (id == LOEFFECT_STANCE_ID_##NAME) return #name;

  LOEFFECT_STANCE_EACH(each_);

  assert(false);
  return NULL;

# undef each_
}

bool loeffect_stance_unstringify(
    loeffect_stance_id_t* id, const char* str, size_t len) {
  assert(id != NULL);
  assert(str != NULL || len == 0);

# define each_(NAME, name) do {\
    if (strncmp(str, #name, len) == 0 && #name[len] == 0) {  \
      *id = LOEFFECT_STANCE_ID_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOEFFECT_STANCE_EACH(each_);
  return false;

# undef each_
}

loshader_menu_stance_id_t loeffect_stance_get_id_for_menu_shader(
    loeffect_stance_id_t id) {
# define each_(NAME, name) do {\
    if (id == LOEFFECT_STANCE_ID_##NAME) {  \
      return LOSHADER_MENU_STANCE_ID_##NAME;  \
    }  \
  } while (0)

  LOEFFECT_STANCE_EACH(each_);

  assert(false);
  return LOSHADER_MENU_STANCE_ID_EMPTY;

# undef each_
}

void loeffect_stance_set_initialize(loeffect_stance_set_t* set) {
  assert(set != NULL);

  *set = 1 << LOEFFECT_STANCE_ID_MISSIONARY;
}

void loeffect_stance_set_deinitialize(loeffect_stance_set_t* set) {
  assert(set != NULL);

}

void loeffect_stance_set_add(
    loeffect_stance_set_t* set, loeffect_stance_id_t id) {
  assert(set != NULL);

  *set |= 1 << id;
}

void loeffect_stance_set_remove(
    loeffect_stance_set_t* set, loeffect_stance_id_t id) {
  assert(set != NULL);

  *set &= ~(1 << id);
}

bool loeffect_stance_set_has(
    const loeffect_stance_set_t* set, loeffect_stance_id_t id) {
  assert(set != NULL);

  return *set & (1 << id);
}

void loeffect_stance_set_affect_base_status(
    const loeffect_stance_set_t* set,
    loeffect_recipient_status_t* status) {
  assert(set    != NULL);
  assert(status != NULL);

}

void loeffect_stance_set_pack(
    const loeffect_stance_set_t* set, msgpack_packer* packer) {
  assert(set    != NULL);
  assert(packer != NULL);

  loeffect_stance_id_t mask = 1;
  size_t len = 0;
  while (mask <= *set) {
    len += !!(*set & mask);
    mask <<= 1;
  }
  msgpack_pack_array(packer, len);

  mask = 1;
  size_t i = 0;
  while (*set >= mask) {
    if (*set & mask) {
      mpkutil_pack_str(packer, loeffect_stance_stringify(i));
    }
    ++i;
    mask <<= 1;
  }
}

bool loeffect_stance_set_unpack(
    loeffect_stance_set_t* set, const msgpack_object* obj) {
  assert(set != NULL);

  const msgpack_object_array* array = mpkutil_get_array(obj);
  if (array == NULL) return false;

  for (size_t i = 0; i < array->size; ++i) {
    size_t      len;
    const char* name;
    if (!mpkutil_get_str(&array->ptr[i], &name, &len)) continue;

    loeffect_stance_id_t stance;
    if (!loeffect_stance_unstringify(&stance, name, len)) continue;
    *set |= 1 << stance;
  }
  return true;
}
