#include "./abchash.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define CHAOS_ABCHASH_GET_CODE(c) (  \
      ('0' <= c && c <= '9')? c-'0':      \
      ('a' <= c && c <= 'z')? c-'a'+10:   \
      (c == '_')       ? 36: UINT8_MAX)

#define CHAOS_ABCHASH_MAX_CODE 36

uint64_t chaos_abchash(const char* str, size_t len) {
  assert(chaos_abchash_validate(str, len));

  uint64_t ans = 0;
  for (size_t i = 0; i < len; ++i) {
    ans = ans*(CHAOS_ABCHASH_MAX_CODE+1) + CHAOS_ABCHASH_GET_CODE(str[i]);
  }
  return ans;
}

bool chaos_abchash_validate(const char* str, size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (CHAOS_ABCHASH_GET_CODE(str[i]) > CHAOS_ABCHASH_MAX_CODE) {
      return false;
    }
  }
  return len > 0;
}
