#include "./charcode.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

size_t conv_charcode_utf8_to_utf32(uint32_t* c, const char* s, size_t len) {
  assert(c != NULL);
  assert(s != NULL);

  if ((*s & 0x80) == 0x00) {
    if (len < 1) return 0;
    *c = s[0];
    return 1;
  }
  if ((*s & 0xE0) == 0xC0) {
    if (len < 2) return 0;
    *c = (s[0] & 0x1F) << 6 | (s[1] & 0x3F);
    return 2;
  }
  if ((*s & 0xF0) == 0xE0) {
    if (len < 3) return 0;
    *c = (s[0] & 0x0F) << 12 | (s[1] & 0x3F) << 6 | (s[2] & 0x3F);
    return 3;
  }
  if ((*s & 0xF8) == 0xF0) {
    if (len < 4) return 0;
    *c =
        (s[0] & 0x07) << 18 |
        (s[1] & 0x3F) << 12 |
        (s[2] & 0x3F) <<  6 |
        (s[3] & 0x3F);
    return 4;
  }
  return 0;
}
