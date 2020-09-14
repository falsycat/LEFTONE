#include "./get.h"

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

bool mpkutil_get_bool(const msgpack_object* obj, bool* b) {
  if (obj == NULL) return false;
  if (obj->type != MSGPACK_OBJECT_BOOLEAN) return false;
  if (b != NULL) *b = obj->via.boolean;
  return true;
}

bool mpkutil_get_int(const msgpack_object* obj, intmax_t* i) {
  if (obj == NULL) return false;
  if (obj->type != MSGPACK_OBJECT_POSITIVE_INTEGER &&
      obj->type != MSGPACK_OBJECT_NEGATIVE_INTEGER) {
    return false;
  }
  if (i != NULL) *i = obj->via.i64;
  return true;
}

#define define_mpkutil_get_intN_(N)  \
    bool mpkutil_get_int##N(const msgpack_object* obj, int##N##_t* i) {  \
      intmax_t temp;  \
      if (!mpkutil_get_int(obj, &temp) ||  \
          temp < INT##N##_MIN || temp > INT##N##_MAX) {  \
        return false;  \
      }  \
      if (i != NULL) *i = temp;  \
      return true;  \
    }
define_mpkutil_get_intN_(8);
define_mpkutil_get_intN_(16);
define_mpkutil_get_intN_(32);
define_mpkutil_get_intN_(64);
#undef define_mpkutil_get_intN_

bool mpkutil_get_uint(const msgpack_object* obj, uintmax_t* i) {
  if (obj == NULL || obj->type != MSGPACK_OBJECT_POSITIVE_INTEGER) {
    return false;
  }
  if (i != NULL) *i = obj->via.u64;
  return true;
}

#define define_mpkutil_get_uintN_(N)  \
    bool mpkutil_get_uint##N(const msgpack_object* obj, uint##N##_t* i) {  \
      uintmax_t temp;  \
      if (!mpkutil_get_uint(obj, &temp) || temp > UINT##N##_MAX) {  \
        return false;  \
      }  \
      if (i != NULL) *i = temp;  \
      return true;  \
    }
define_mpkutil_get_uintN_(8);
define_mpkutil_get_uintN_(16);
define_mpkutil_get_uintN_(32);
define_mpkutil_get_uintN_(64);
#undef define_mpkutil_get_uintN_

bool mpkutil_get_float(const msgpack_object* obj, float* f) {
  if (obj == NULL || obj->type != MSGPACK_OBJECT_FLOAT64 ||
      !MATH_FLOAT_VALID(obj->via.f64)) {
    return false;
  }
  if (f != NULL) *f = obj->via.f64;
  return true;
}

bool mpkutil_get_str(const msgpack_object* obj, const char** s, size_t* len) {
  if (obj == NULL || obj->type != MSGPACK_OBJECT_STR) {
    return false;
  }
  if (s   != NULL) *s   = obj->via.str.ptr;
  if (len != NULL) *len = obj->via.str.size;
  return true;
}

bool mpkutil_get_vec2(const msgpack_object* obj, vec2_t* v) {
  const msgpack_object_array* array = mpkutil_get_array(obj);
  if (array == NULL || array->size != 2) return false;
  for (size_t i = 0; i < 2; ++i) {
    if (array->ptr[i].type != MSGPACK_OBJECT_FLOAT64) return false;
  }
  if (v != NULL) {
    v->x = array->ptr[0].via.f64;
    v->y = array->ptr[1].via.f64;
  }
  return true;
}

bool mpkutil_get_vec4(const msgpack_object* obj, vec4_t* v) {
  const msgpack_object_array* array = mpkutil_get_array(obj);
  if (array == NULL || array->size != 4) return false;
  for (size_t i = 0; i < 4; ++i) {
    if (array->ptr[i].type != MSGPACK_OBJECT_FLOAT64) return false;
  }
  if (v != NULL) {
    v->x = array->ptr[0].via.f64;
    v->y = array->ptr[1].via.f64;
    v->y = array->ptr[2].via.f64;
    v->y = array->ptr[3].via.f64;
  }
  return true;
}

const msgpack_object_array* mpkutil_get_array(const msgpack_object* obj) {
  if (obj == NULL || obj->type != MSGPACK_OBJECT_ARRAY) {
    return NULL;
  }
  return &obj->via.array;
}

const msgpack_object_map* mpkutil_get_map(const msgpack_object* obj) {
  if (obj == NULL || obj->type != MSGPACK_OBJECT_MAP) {
    return NULL;
  }
  return &obj->via.map;
}

const msgpack_object* mpkutil_get_map_item_by_str(
    const msgpack_object_map* map, const char* name) {
  if (map == NULL) return NULL;

  for (size_t i = 0; i < map->size; ++i) {
    const msgpack_object* key_obj = &map->ptr[i].key;
    if (key_obj->type != MSGPACK_OBJECT_STR) continue;

    const msgpack_object_str* key = &key_obj->via.str;
    if (strncmp(key->ptr, name, key->size) == 0 && name[key->size] == 0) {
      return &map->ptr[i].val;
    }
  }
  return NULL;
}
