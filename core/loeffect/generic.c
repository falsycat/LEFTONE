#include "./generic.h"

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

void loeffect_generic_immediate_param_pack(
    const loeffect_generic_immediate_param_t* param,
    msgpack_packer*                           packer) {
  assert(param  != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "amount");
  msgpack_pack_double(packer, param->amount);
}

bool loeffect_generic_immediate_param_unpack(
    loeffect_generic_immediate_param_t* param,
    const msgpack_object*               obj) {
  assert(param != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

  const msgpack_object* amount = mpkutil_get_map_item_by_str(root, "amount");
  if (!mpkutil_get_float(amount, &param->amount)) {
    return false;
  }
  return true;
}

void loeffect_generic_lasting_param_pack(
    const loeffect_generic_lasting_param_t* param, msgpack_packer* packer) {
  assert(param  != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 3);

  mpkutil_pack_str(packer, "start");
  msgpack_pack_uint64(packer, param->start);

  mpkutil_pack_str(packer, "duration");
  msgpack_pack_uint64(packer, param->duration);

  mpkutil_pack_str(packer, "amount");
  msgpack_pack_double(packer, param->amount);
}

bool loeffect_generic_lasting_param_unpack(
    loeffect_generic_lasting_param_t* param, const msgpack_object* obj) {
  assert(param != NULL);
  assert(obj   != NULL);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

  if (!mpkutil_get_uint64(item_("start"), &param->start)) {
    return false;
  }
  if (!mpkutil_get_uint64(item_("duration"), &param->duration)) {
    return false;
  }
  if (!mpkutil_get_float(item_("amount"), &param->amount)) {
    return false;
  }

# undef item_
  return true;
}
