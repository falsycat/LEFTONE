#include "./file.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <msgpack.h>

bool mpkutil_file_unpack_with_unpacker(
    msgpack_unpacked* obj,
    FILE*             fp,
    msgpack_unpacker* unpacker) {
  assert(obj      != NULL);
  assert(fp       != NULL);
  assert(unpacker != NULL);

  for (;;) {
    if (feof(fp)) return false;

    const size_t maxlen = msgpack_unpacker_buffer_capacity(unpacker);
    if (maxlen == 0) {
      fprintf(stderr, "unpacker buffer overflow\n");
      abort();
    }

    const size_t len = fread(
        msgpack_unpacker_buffer(unpacker), 1, maxlen, fp);
    if (ferror(fp)) return false;

    msgpack_unpacker_buffer_consumed(unpacker, len);

    size_t parsed_len;
    switch (msgpack_unpacker_next_with_size(unpacker, obj, &parsed_len)) {
    case MSGPACK_UNPACK_SUCCESS:
      return fseek(fp, (long) parsed_len - len, SEEK_CUR) == 0;
    case MSGPACK_UNPACK_CONTINUE:
      break;

    case MSGPACK_UNPACK_PARSE_ERROR:
    default:
      return false;
    }
  }
}

bool mpkutil_file_unpack(msgpack_unpacked* obj, FILE* fp) {
  assert(fp  != NULL);
  assert(obj != NULL);

  msgpack_unpacker unpacker;
  if (!msgpack_unpacker_init(&unpacker, 1024)) return false;

  const bool result = mpkutil_file_unpack_with_unpacker(obj, fp, &unpacker);
  msgpack_unpacker_destroy(&unpacker);
  return result;
}
