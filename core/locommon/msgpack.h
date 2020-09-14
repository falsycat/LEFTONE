#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/counter.h"
#include "core/locommon/null.h"

/* THE FOLLOWING INCLUDES DESTROY DEPENDENCY STRUCTURE BETWEEN MODULES. :( */
#include "core/loeffect/effect.h"
#include "core/loeffect/generic.h"
#include "core/loeffect/recipient.h"
#include "core/loeffect/stance.h"
#include "core/loentity/entity.h"

#define LOCOMMON_MSGPACK_PACK_ANY_(packer, v) _Generic((v),  \
      const int32_t*:  locommon_msgpack_pack_int32_,  \
      const uint64_t*: locommon_msgpack_pack_uint64_,  \
      const float*:    locommon_msgpack_pack_float_,  \
      const bool*:     locommon_msgpack_pack_bool_,  \
      const char*:     locommon_msgpack_pack_str_,  \
      const vec2_t*:   locommon_msgpack_pack_vec2_,  \
      const vec4_t*:   locommon_msgpack_pack_vec4_,  \
  \
      const locommon_counter_t*:  locommon_counter_pack,  \
      const locommon_null_t*:     locommon_null_pack,  \
      const locommon_position_t*: locommon_position_pack,  \
      const locommon_ticker_t*:   locommon_ticker_pack,  \
  \
      const loeffect_t*: loeffect_pack,  \
      const loeffect_generic_immediate_param_t*: loeffect_generic_immediate_param_pack,  \
      const loeffect_generic_lasting_param_t*: loeffect_generic_lasting_param_pack,  \
      const loeffect_recipient_effect_param_t*: loeffect_recipient_effect_param_pack,  \
      const loeffect_stance_set_t*: loeffect_stance_set_pack  \
    )(v, packer)

#define LOCOMMON_MSGPACK_PACK_ANY(packer, v)  \
    LOCOMMON_MSGPACK_PACK_ANY_(packer, (const typeof(*v)*) (v))

#define LOCOMMON_MSGPACK_UNPACK_ANY(obj, v) _Generic((v),  \
      int32_t*:  locommon_msgpack_unpack_int32_,  \
      uint64_t*: locommon_msgpack_unpack_uint64_,  \
      float*:    locommon_msgpack_unpack_float_,  \
      bool*:     locommon_msgpack_unpack_bool_,  \
      vec2_t*:   locommon_msgpack_unpack_vec2_,  \
      vec4_t*:   locommon_msgpack_unpack_vec4_,  \
  \
      locommon_counter_t*:  locommon_counter_unpack,  \
      locommon_null_t*:     locommon_null_unpack,  \
      locommon_position_t*: locommon_position_unpack,  \
      locommon_ticker_t*:   locommon_ticker_unpack,  \
  \
      loeffect_t*: loeffect_unpack,  \
      loeffect_generic_immediate_param_t*: loeffect_generic_immediate_param_unpack,  \
      loeffect_generic_lasting_param_t*: loeffect_generic_lasting_param_unpack,  \
      loeffect_recipient_effect_param_t*: loeffect_recipient_effect_param_unpack,  \
      loeffect_stance_set_t*: loeffect_stance_set_unpack  \
    )(v, obj)

static inline void locommon_msgpack_pack_int32_(
    const int32_t* v, msgpack_packer* packer) {
  msgpack_pack_int32(packer, *v);
}
static inline void locommon_msgpack_pack_uint64_(
    const uint64_t* v, msgpack_packer* packer) {
  msgpack_pack_uint64(packer, *v);
}
static inline void locommon_msgpack_pack_float_(
    const float* v, msgpack_packer* packer) {
  msgpack_pack_double(packer, *v);
}
static inline void locommon_msgpack_pack_bool_(
    const bool* v, msgpack_packer* packer) {
  mpkutil_pack_bool(packer, *v);
}
static inline void locommon_msgpack_pack_str_(
    const char* str, msgpack_packer* packer) {
  mpkutil_pack_str(packer, str);
}
static inline void locommon_msgpack_pack_vec2_(
    const vec2_t* v, msgpack_packer* packer) {
  mpkutil_pack_vec2(packer, v);
}
static inline void locommon_msgpack_pack_vec4_(
    const vec4_t* v, msgpack_packer* packer) {
  mpkutil_pack_vec4(packer, v);
}

static inline bool locommon_msgpack_unpack_int32_(
    int32_t* v, const msgpack_object* obj) {
  return mpkutil_get_int32(obj, v);
}
static inline bool locommon_msgpack_unpack_uint64_(
    uint64_t* v, const msgpack_object* obj) {
  return mpkutil_get_uint64(obj, v);
}
static inline bool locommon_msgpack_unpack_float_(
    float* v, const msgpack_object* obj) {
  return mpkutil_get_float(obj, v);
}
static inline bool locommon_msgpack_unpack_bool_(
    bool* v, const msgpack_object* obj) {
  return mpkutil_get_bool(obj, v);
}
static inline bool locommon_msgpack_unpack_vec2_(
    vec2_t* v, const msgpack_object* obj) {
  return mpkutil_get_vec2(obj, v);
}
static inline bool locommon_msgpack_unpack_vec4_(
    vec4_t* v, const msgpack_object* obj) {
  return mpkutil_get_vec4(obj, v);
}
