#pragma once

#include <stdint.h>

typedef uint16_t statman_state_t;

typedef struct statman_meta_t statman_meta_t;
struct statman_meta_t {
  statman_state_t state;
  const char*     name;
  const void*     data;

  void
  (*initialize)(
      const statman_meta_t* meta,
      void*                 instance,
      statman_state_t*      next
  );
  void
  (*update)(
      const statman_meta_t* meta,
      void*                 instance,
      statman_state_t*      next
  );
  void
  (*finalize)(
      const statman_meta_t* meta,
      void*                 instance,
      statman_state_t*      next
  );
};

void
statman_update(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      state
);

/* don't call in callback functions */
void
statman_transition_to(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      state,
    statman_state_t       next
);
