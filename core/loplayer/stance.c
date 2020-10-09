#include "./stance.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/loshader/menu_stance.h"

loshader_menu_stance_id_t loplayer_stance_get_menu_shader_id(
    loplayer_stance_t stance) {
# define each_(NAME, name) do {\
    if (stance == LOPLAYER_STANCE_##NAME) {  \
      return LOSHADER_MENU_STANCE_ID_##NAME;  \
    }  \
  } while (0)

  LOPLAYER_STANCE_EACH(each_);

  assert(false);
  return LOSHADER_MENU_STANCE_ID_EMPTY;

# undef each_
}

void loplayer_stance_set_initialize(loplayer_stance_set_t* set) {
  assert(set != NULL);

  *set = 0;
}

void loplayer_stance_set_deinitialize(loplayer_stance_set_t* set) {
  assert(set != NULL);

}

void loplayer_stance_set_add(
    loplayer_stance_set_t* set, loplayer_stance_t stance) {
  assert(set != NULL);

  *set |= 1 << stance;
}

void loplayer_stance_set_remove(
    loplayer_stance_set_t* set, loplayer_stance_t stance) {
  assert(set != NULL);

  *set &= ~(1 << stance);
}

bool loplayer_stance_set_has(
    const loplayer_stance_set_t* set, loplayer_stance_t stance) {
  assert(set != NULL);

  return *set & (1 << stance);
}

void loplayer_stance_set_pack(
    const loplayer_stance_set_t* set, msgpack_packer* packer) {
  assert(set    != NULL);
  assert(packer != NULL);

  loplayer_stance_t mask = 1;
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
      mpkutil_pack_str(packer, loplayer_stance_stringify(i));
    }
    ++i;
    mask <<= 1;
  }
}

bool loplayer_stance_set_unpack(
    loplayer_stance_set_t* set, const msgpack_object* obj) {
  assert(set != NULL);

  const msgpack_object_array* array = mpkutil_get_array(obj);
  if (array == NULL) return false;

  for (size_t i = 0; i < array->size; ++i) {
    size_t      len;
    const char* name;
    if (!mpkutil_get_str(&array->ptr[i], &name, &len)) continue;

    loplayer_stance_t stance;
    if (!loplayer_stance_unstringify(&stance, name, len)) continue;
    *set |= 1 << stance;
  }
  return true;
}
