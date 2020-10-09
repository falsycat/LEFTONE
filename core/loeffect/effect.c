#include "./effect.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "core/locommon/msgpack.h"

#include "./generic.h"

const char* loeffect_id_stringify(loeffect_id_t id) {
# define each_(NAME, s, d) do {  \
    if (LOEFFECT_ID_##NAME == id) return s;  \
  } while(0)

  LOEFFECT_ID_EACH_(each_);

  assert(false);
  return NULL;

# undef each_
}

bool loeffect_id_unstringify(loeffect_id_t* id, const char* str, size_t len) {
  assert(id != NULL);
  assert(str != NULL || len == 0);

# define each_(NAME, s, d) do {  \
    if (strncmp(str, s, len) == 0 && s[len] == 0) {  \
      *id = LOEFFECT_ID_##NAME;  \
      return true;  \
    }  \
  } while (0)

  LOEFFECT_ID_EACH_(each_);
  return false;

# undef each_
}

void loeffect_pack(const loeffect_t* effect, msgpack_packer* packer) {
  assert(effect != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 2);

  mpkutil_pack_str(packer, "id");
  mpkutil_pack_str(packer, loeffect_id_stringify(effect->id));

  mpkutil_pack_str(packer, "data");

# define each_(NAME, s, d) do {  \
    if (effect->id == LOEFFECT_ID_##NAME) {  \
      LOCOMMON_MSGPACK_PACK_ANY(packer, &effect->data.d);  \
    }  \
  } while (0)

  LOEFFECT_ID_EACH_(each_);

# undef each_
}

bool loeffect_unpack(loeffect_t* effect, const msgpack_object* obj) {
  assert(effect != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

  const char* idstr;
  size_t      idstr_len;
  if (!mpkutil_get_str(item_("id"), &idstr, &idstr_len) ||
      !loeffect_id_unstringify(&effect->id, idstr, idstr_len)) {
    return false;
  }

# define each_(NAME, s, d) do {  \
    if (effect->id == LOEFFECT_ID_##NAME) {  \
      return LOCOMMON_MSGPACK_UNPACK_ANY(item_("data"), &effect->data.d);  \
    }  \
  } while (0)

  LOEFFECT_ID_EACH_(each_);

# undef each_
  return false;
}
