#include "./memory.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __linux__
# include <execinfo.h>
#endif

#define MEMORY_LOG_FILE "memory-trace"

void* memory_new(size_t size) {
  if (size == 0) return NULL;

  void* ptr = malloc(size);
  if (ptr == NULL) {
    fprintf(stderr, "failed to allocate %zu bytes memory", size);
    abort();
  }

  memory_log("new 0x%tx (%zu bytes)", (ptrdiff_t) ptr, size);
  return ptr;
}

void* memory_resize(void* ptr, size_t size) {
  if (ptr  == NULL) return memory_new(size);
  if (size == 0)    return ptr;

  void* newptr = realloc(ptr, size);
  if (newptr == NULL) {
    fprintf(stderr,
      "failed to allocate %zu bytes memory for resizing 0x%tx",
      size, (ptrdiff_t) ptr);
    abort();
  }

  memory_log("resize 0x%tx -> 0x%tx (%zu bytes)",
      (ptrdiff_t) ptr, (ptrdiff_t) newptr, size);
  return newptr;
}

void memory_delete(void* ptr) {
  if (ptr == NULL) return;
  free(ptr);

  memory_log("delete 0x%tx", (ptrdiff_t) ptr);
}

#ifndef NDEBUG
  void memory_log(const char* fmt, ...) {
    static FILE* fp = NULL;

    if (fp == NULL) {
      fp = fopen(MEMORY_LOG_FILE, "w");
      assert(fp != NULL);
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);

    fprintf(fp, "\n");

#   if __linux__
      void* callstack[32];
      const size_t frames  = backtrace(callstack, sizeof(callstack));
      char**       symbols = backtrace_symbols(callstack, frames);
      for (size_t i = 0; i < frames; ++i) {
        fprintf(fp, "  %s\n", symbols[i]);
      }
      free(symbols);
#   endif  /* __linux__ */

    fprintf(fp, "\n");
    fflush(fp);
  }
#endif  /* NDEBUG */
