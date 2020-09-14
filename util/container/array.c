#include "./array.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "util/memory/memory.h"

typedef struct {
  size_t length;
  size_t reserved;
} container_array_header_t;

void container_array_delete(void* array) {
  if (array == NULL) return;
  memory_delete(array - sizeof(container_array_header_t));
}

void container_array_reserve_(void** array, size_t elmlen, size_t len) {
  assert(array != NULL);

  if (len == 0) return;

  container_array_header_t* ptr;
  if (*array == NULL) {
    ptr = memory_new(sizeof(*ptr) + len*elmlen);
    *ptr = (typeof(*ptr)) {
      .length   = 0,
      .reserved = len,
    };
  } else {
    ptr = *array - sizeof(*ptr);
    if (ptr->reserved >= len) return;

    ptr = memory_resize(ptr, sizeof(*ptr) + len*elmlen);
    ptr->reserved = len;
  }
  *array = ptr + 1;
}

void container_array_resize_(void** array, size_t elmlen, size_t len) {
  assert(array != NULL);

  container_array_reserve_(array, elmlen, len);
  if (*array == NULL) return;

  container_array_header_t* ptr = *array - sizeof(*ptr);
  ptr->length = len;
}

void container_array_insert_(void** array, size_t elmlen, size_t index) {
  assert(array != NULL);

  const size_t len = container_array_get_length(*array) + 1;
  assert(index < len);
  container_array_resize_(array, elmlen, len);

  uint8_t* src = *array + (index)*elmlen;
  memmove(src + elmlen, src, (len-index-1)*elmlen);
}

void container_array_remove_(void** array, size_t elmlen, size_t index) {
  const size_t oldlen = container_array_get_length(*array);
  assert(index < oldlen);

  const size_t len = oldlen-1;

  uint8_t* dst = *array + index*elmlen;
  memmove(dst, dst + elmlen, (len-index)*elmlen);
  container_array_resize_(array, elmlen, len);
}

void* container_array_duplicate_(const void* array, size_t elmlen) {
  if (array == NULL) return NULL;
  const container_array_header_t* ptr = array - sizeof(*ptr);

  const size_t sz = sizeof(*ptr) + ptr->length*elmlen;

  container_array_header_t* ptrdup = memory_new(sz);
  memcpy(ptrdup, ptr, sz);
  ptrdup->reserved = ptr->length;
  return (void*) (ptrdup + 1);
}

size_t container_array_get_length_(const void* array) {
  if (array == NULL) return 0;
  const container_array_header_t* ptr = array - sizeof(*ptr);
  return ptr->length;
}
