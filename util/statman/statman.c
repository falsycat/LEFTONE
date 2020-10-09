#include "./statman.h"

#include <assert.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define REDIRECT_MAX_ 100

static const statman_meta_t* statman_get_meta_by_state_(
    const statman_meta_t* meta, statman_state_t state) {
  assert(meta  != NULL);

  for (size_t i = 0; meta[i].name != NULL; ++i) {
    if (meta[i].state == state) return &meta[i];
  }
  fprintf(stderr, "statman: unknown state %"PRIu16"\n", state);
  abort();
}

static void statman_initialize_state_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      state,
    size_t                depth) {
  assert(meta  != NULL);
  assert(state != NULL);
  assert(depth <  REDIRECT_MAX_);

  const statman_meta_t* m = statman_get_meta_by_state_(meta, *state);
  if (m->initialize == NULL) return;

  const statman_state_t backup = *state;
  m->initialize(m, instance, state);

  if (backup != *state) {
    statman_initialize_state_(meta, instance, state, depth+1);
  }
}

static void statman_update_state_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      state,
    size_t                depth) {
  assert(meta  != NULL);
  assert(state != NULL);
  assert(depth <  REDIRECT_MAX_);

  const statman_meta_t* m = statman_get_meta_by_state_(meta, *state);
  if (m->update == NULL) return;

  const statman_state_t backup = *state;
  m->update(m, instance, state);

  if (backup != *state) {
    statman_initialize_state_(meta, instance, state, depth+1);
    statman_update_state_(meta, instance, state, depth+1);
  }
}

void statman_update(
    const statman_meta_t* meta, void* instance, statman_state_t* state) {
  assert(meta  != NULL);
  assert(state != NULL);

  statman_update_state_(meta, instance, state, 0);
}

void statman_transition_to(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      state,
    statman_state_t       next) {
  assert(meta  != NULL);
  assert(state != NULL);

  if (*state == next) return;

  const statman_meta_t* m = statman_get_meta_by_state_(meta, *state);
  if (m->finalize != NULL) {
    m->finalize(m, instance, &next);
  }
  if (*state == next) return;

  *state = next;
  statman_initialize_state_(meta, instance, state, 0);
}
