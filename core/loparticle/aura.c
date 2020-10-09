#include "./aura.h"

#include <assert.h>
#include <stdbool.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/loentity/entity.h"

#include "./base.h"

/* generated serializer */
#include "core/loparticle/crial/aura.h"

_Static_assert(
    sizeof(loparticle_aura_param_t) <= LOPARTICLE_BASE_DATA_MAX_SIZE,
    "data size overflow");

bool loparticle_aura_param_valid(const loparticle_aura_param_t* param) {
  return param != NULL;
}

void loparticle_aura_param_pack(
    const loparticle_aura_param_t* param, msgpack_packer* packer) {
  assert(param  != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, CRIAL_PROPERTY_COUNT_);

  CRIAL_SERIALIZER_;
}

bool loparticle_aura_param_unpack(
    loparticle_aura_param_t* param, const msgpack_object* obj) {
  assert(param != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);
  if (root == NULL) return false;

  CRIAL_DESERIALIZER_;

  return loparticle_aura_param_valid(param);
}

void loparticle_aura_build(
    loparticle_base_t*             base,
    loparticle_type_t              type,
    const loparticle_aura_param_t* param) {
  assert(base  != NULL);
  assert(param != NULL);

  base->type = type;
  *((loparticle_aura_param_t*) base->data) = *param;
}

bool loparticle_aura_guard_update(loparticle_base_t* base) {
  assert(base != NULL);

  /* TODO(catfoot):  */

  return true;
}
