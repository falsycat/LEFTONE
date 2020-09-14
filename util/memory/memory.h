#pragma once

#include <stddef.h>

void*
memory_new(
    size_t size
);

void*
memory_resize(
    void*  ptr,
    size_t size
);

void
memory_delete(
    void* ptr
);

#ifndef NDEBUG
  void
  memory_log(
      const char* fmt,
      ...
  );
#else
# define memory_log(fmt, ...) (void) fmt
#endif  /* NDEBUG */
