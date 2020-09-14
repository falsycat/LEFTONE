#include "./pack.h"

#include <assert.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/math/vector.h"

void mpkutil_pack_bool(msgpack_packer* packer, bool b) {
  assert(packer != NULL);

  (b? msgpack_pack_true: msgpack_pack_false)(packer);
}

void mpkutil_pack_str(msgpack_packer* packer, const char* str) {
  assert(packer != NULL);
  assert(str    != NULL);

  mpkutil_pack_strn(packer, str, strlen(str));
}

void mpkutil_pack_strn(msgpack_packer* packer, const char* str, size_t len) {
  assert(packer != NULL);
  assert(str    != NULL);

  msgpack_pack_str(packer, len);
  msgpack_pack_str_body(packer, str, len);
}

void mpkutil_pack_vec2(msgpack_packer* packer, const vec2_t* v) {
  assert(packer != NULL);
  assert(v      != NULL);

  msgpack_pack_array(packer, 2);
  msgpack_pack_double(packer, v->x);
  msgpack_pack_double(packer, v->y);
}

void mpkutil_pack_vec4(msgpack_packer* packer, const vec4_t* v) {
  assert(packer != NULL);
  assert(v      != NULL);

  msgpack_pack_array(packer, 4);
  msgpack_pack_double(packer, v->x);
  msgpack_pack_double(packer, v->y);
  msgpack_pack_double(packer, v->z);
  msgpack_pack_double(packer, v->w);
}
