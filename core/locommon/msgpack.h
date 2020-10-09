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
#include "core/lobullet/type.h"
#include "core/lochara/state.h"
#include "core/lochara/strategy.h"
#include "core/lochara/type.h"
#include "core/loeffect/effect.h"
#include "core/loeffect/generic.h"
#include "core/loeffect/recipient.h"
#include "core/loground/type.h"
#include "core/loparticle/misc.h"
#include "core/loresource/music.h"
#include "core/loresource/sound.h"

#define LOCOMMON_MSGPACK_PACK_ANY_(packer, v) _Generic((v),  \
      const int32_t*:  locommon_msgpack_pack_int32_,  \
      const uint64_t*: locommon_msgpack_pack_uint64_,  \
      const float*:    locommon_msgpack_pack_float_,  \
      const bool*:     locommon_msgpack_pack_bool_,  \
      const char*:     locommon_msgpack_pack_str_,  \
      const vec2_t*:   locommon_msgpack_pack_vec2_,  \
      const vec4_t*:   locommon_msgpack_pack_vec4_,  \
  \
      const lobullet_type_t*:  locommon_msgpack_lobullet_type_pack_,  \
  \
      const lochara_type_t*:     locommon_msgpack_lochara_type_pack_,  \
      const lochara_state_t*:    locommon_msgpack_lochara_state_pack_,  \
      const lochara_strategy_t*: locommon_msgpack_lochara_strategy_pack_,  \
  \
      const locommon_counter_t*:  locommon_counter_pack,  \
      const locommon_null_t*:     locommon_null_pack,  \
      const locommon_position_t*: locommon_position_pack,  \
      const locommon_ticker_t*:   locommon_ticker_pack,  \
  \
      const loeffect_id_t*:                      locommon_msgpack_loeffect_id_pack_,  \
      const loeffect_t*:                         loeffect_pack,  \
      const loeffect_generic_immediate_param_t*: loeffect_generic_immediate_param_pack,  \
      const loeffect_generic_lasting_param_t*:   loeffect_generic_lasting_param_pack,  \
      const loeffect_recipient_t*:               loeffect_recipient_pack,  \
  \
      const loground_type_t*: locommon_msgpack_loground_type_pack_,  \
  \
      const loparticle_type_t*: locommon_msgpack_loparticle_type_pack_,  \
  \
      const loresource_music_id_t*: locommon_msgpack_loresource_music_id_pack_,  \
      const loresource_sound_id_t*: locommon_msgpack_loresource_sound_id_pack_  \
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
      lobullet_type_t*:  locommon_msgpack_lobullet_type_unpack_,  \
  \
      lochara_type_t*:     locommon_msgpack_lochara_type_unpack_,  \
      lochara_state_t*:    locommon_msgpack_lochara_state_unpack_,  \
      lochara_strategy_t*: locommon_msgpack_lochara_strategy_unpack_,  \
  \
      locommon_counter_t*:  locommon_counter_unpack,  \
      locommon_null_t*:     locommon_null_unpack,  \
      locommon_position_t*: locommon_position_unpack,  \
      locommon_ticker_t*:   locommon_ticker_unpack,  \
  \
      loeffect_id_t*:                      locommon_msgpack_loeffect_id_unpack_,  \
      loeffect_t*:                         loeffect_unpack,  \
      loeffect_generic_immediate_param_t*: loeffect_generic_immediate_param_unpack,  \
      loeffect_generic_lasting_param_t*:   loeffect_generic_lasting_param_unpack,  \
      loeffect_recipient_t*:               loeffect_recipient_unpack,  \
  \
      loground_type_t*: locommon_msgpack_loground_type_unpack_,  \
  \
      loparticle_type_t*: locommon_msgpack_loparticle_type_unpack_,  \
  \
      loresource_music_id_t*: locommon_msgpack_loresource_music_id_unpack_,  \
      loresource_sound_id_t*: locommon_msgpack_loresource_sound_id_unpack_  \
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

#define enum_packer_(name)  \
    static inline void locommon_msgpack_##name##_pack_(  \
        const name##_t* v, msgpack_packer* packer) {  \
      mpkutil_pack_str(packer, name##_stringify(*v));  \
    }

#define enum_unpacker_(name)  \
    static inline bool locommon_msgpack_##name##_unpack_(  \
        name##_t* v, const msgpack_object* obj) {  \
      const char* str;  \
      size_t len;  \
      return  \
          mpkutil_get_str(obj, &str, &len) &&  \
          name##_unstringify(v, str, len);  \
    }

enum_packer_(lobullet_type);
enum_unpacker_(lobullet_type);

enum_packer_(lochara_state);
enum_unpacker_(lochara_state);

enum_packer_(lochara_strategy);
enum_unpacker_(lochara_strategy);

enum_packer_(lochara_type);
enum_unpacker_(lochara_type);

enum_packer_(loeffect_id);
enum_unpacker_(loeffect_id);

enum_packer_(loground_type);
enum_unpacker_(loground_type);

enum_packer_(loparticle_type);
enum_unpacker_(loparticle_type);

enum_packer_(loresource_music_id);
enum_unpacker_(loresource_music_id);

enum_packer_(loresource_sound_id);
enum_unpacker_(loresource_sound_id);

#undef enum_packer_
#undef enum_unpacker_
