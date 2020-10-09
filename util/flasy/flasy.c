#include "./flasy.h"

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <tinycthread.h>

#include "util/memory/memory.h"

typedef struct {
  atomic_uintptr_t fp;
  atomic_bool      request_flush;
  uint8_t          buf[1];
} flasy_handler_t;

struct flasy_t {
  mtx_t  mtx;
  thrd_t thread;

  size_t bufsz;

  atomic_bool alive;

  size_t  length;
  uint8_t handlers[1];
};

static int flasy_main_(void* srv_) {
  assert(srv_ != NULL);
  flasy_t* srv = srv_;

  const size_t hsize = srv->bufsz + offsetof(flasy_handler_t, buf);
  uint8_t*     end   = srv->handlers + hsize*srv->length;

  for (;;) {
    const bool alive = atomic_load(&srv->alive);

    for (uint8_t* itr = srv->handlers; itr < end; itr += hsize) {
      flasy_handler_t* handler = (typeof(handler)) itr;

      FILE* fp = (typeof(fp)) atomic_load(&handler->fp);
      if (fp != NULL && atomic_load(&handler->request_flush)) {
        /* while request_flush is true and fp isn't NULL,
           no other thread modifies the handler */
        fclose(fp);
        atomic_store(&handler->fp, 0);
      }
    }
    thrd_sleep(&(struct timespec) {
      .tv_sec  = 0,
      .tv_nsec = 10000000,  /* = 10 ms */
    }, NULL);

    if (!alive) break;
  }
  return EXIT_SUCCESS;
}

flasy_t* flasy_new(size_t bufsz, size_t hlen) {
  assert(bufsz > 0);
  assert(hlen  > 0);

  const size_t hsize = bufsz + offsetof(flasy_handler_t, buf);

  flasy_t* srv = memory_new(sizeof(*srv) + hlen*hsize - 1);
  *srv = (typeof(*srv)) {
    .bufsz  = bufsz,
    .length = hlen,
  };

  atomic_store(&srv->alive, true);

  uint8_t* itr = srv->handlers;
  for (size_t i = 0; i < srv->length; ++i) {
    atomic_init(&((flasy_handler_t*) itr)->fp, 0);
    itr += hsize;
  }

  if (mtx_init(&srv->mtx, mtx_plain) != thrd_success) {
    fprintf(stderr, "flasy: failed to create mutex\n");
    abort();
  }
  if (thrd_create(&srv->thread, &flasy_main_, srv) != thrd_success) {
    fprintf(stderr, "flasy: failed to create thread\n");
    abort();
  }
  return srv;
}

void flasy_delete(flasy_t* srv) {
  if (srv == NULL) return;

  atomic_store(&srv->alive, false);
  thrd_join(srv->thread, NULL);

  mtx_destroy(&srv->mtx);

  memory_delete(srv);
}

FILE* flasy_open_file(flasy_t* srv, const char* path, bool binary) {
  assert(srv  != NULL);
  assert(path != NULL);

  const size_t hsize = srv->bufsz + offsetof(flasy_handler_t, buf);

  uint8_t* itr = srv->handlers;
  uint8_t* end = srv->handlers + hsize*srv->length;
  for (; itr < end; itr += hsize) {
    if (atomic_load(&((flasy_handler_t*) itr)->fp) == 0) break;
  }
  if (itr >= end) return NULL;

  FILE* fp = fopen(path, binary? "wb": "w");
  if (fp == NULL) return NULL;

  flasy_handler_t* handler = (typeof(handler)) itr;
  /* while handler->fp is NULL, no other threads modifies the handler */

  setvbuf(fp, (char*) handler->buf, _IOFBF, srv->bufsz);
  atomic_store(&handler->request_flush, false);
  atomic_store(&handler->fp, (uintptr_t) fp);

  return fp;
}

void flasy_close_file(flasy_t* srv, FILE* fp) {
  assert(srv != NULL);
  assert(fp  != NULL);

  const size_t hsize = srv->bufsz + offsetof(flasy_handler_t, buf);

  uint8_t* itr = srv->handlers;
  uint8_t* end = srv->handlers + hsize*srv->length;
  for (; itr < end; itr += hsize) {
    if (atomic_load(&((flasy_handler_t*) itr)->fp) == (uintptr_t) fp) break;
  }
  assert(itr < end);

  flasy_handler_t* handler = (typeof(handler)) itr;
  atomic_store(&handler->request_flush, true);
}
