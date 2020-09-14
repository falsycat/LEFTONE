#include "./null.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

void locommon_null_pack(const locommon_null_t* null, msgpack_packer* packer) {
  assert(null != NULL);
  assert(null != NULL);

  msgpack_pack_nil(packer);
}

bool locommon_null_unpack(locommon_null_t* null, const msgpack_object* obj) {
  assert(null != NULL);

  return obj != NULL;
}
