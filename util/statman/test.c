#undef NDEBUG

#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "./statman.h"

typedef enum {
  CHECK_POINT_NONE,

  CHECK_POINT_STATE0_INITIALIZE,
  CHECK_POINT_STATE1_INITIALIZE,
  CHECK_POINT_STATE2_INITIALIZE,
  CHECK_POINT_STATE3_INITIALIZE,

  CHECK_POINT_STATE0_UPDATE,
  CHECK_POINT_STATE1_UPDATE,
  CHECK_POINT_STATE2_UPDATE,

  CHECK_POINT_STATE0_FINALIZE,
  CHECK_POINT_STATE1_FINALIZE,
  CHECK_POINT_STATE2_FINALIZE,
  CHECK_POINT_STATE3_FINALIZE,
} check_point_t;

static check_point_t actual_[256] = {0};
static size_t        actual_length_ = 0;

static inline void check_point_(check_point_t cp) {
  assert(actual_length_ < sizeof(actual_)/sizeof(actual_[0]));
  actual_[actual_length_++] = cp;
}

static void state_initialize_(
    const statman_meta_t* meta, void* time, statman_state_t* next) {
  assert(meta != NULL);
  assert(time != NULL);
  assert(next != NULL);

  check_point_(CHECK_POINT_STATE0_INITIALIZE+meta->state);
}

static void state3_initialize_(
    const statman_meta_t* meta, void* time, statman_state_t* next) {
  assert(meta != NULL);
  assert(time != NULL);
  assert(next != NULL);

  check_point_(CHECK_POINT_STATE3_INITIALIZE);
  *next = 0;
}

static void state_update_(
    const statman_meta_t* meta, void* time, statman_state_t* next) {
  assert(meta != NULL);
  assert(time != NULL);
  assert(next != NULL);

  check_point_(CHECK_POINT_STATE0_UPDATE+meta->state);

  size_t *t = (typeof(t)) time;
  if ((*t)++%2 == 0) *next = meta->state+1;
}

static void state_finalize_(
    const statman_meta_t* meta, void* time, statman_state_t* next) {
  assert(meta != NULL);
  assert(time != NULL);
  assert(next != NULL);

  check_point_(CHECK_POINT_STATE0_FINALIZE+meta->state);
  *next = 0;
}

static const statman_meta_t table_[] = {
  {
    .state      = 0,
    .name       = "0",
    .initialize = state_initialize_,
    .update     = state_update_,
    .finalize   = state_finalize_,
  },
  {
    .state      = 1,
    .name       = "1",
    .initialize = state_initialize_,
    .update     = state_update_,
    .finalize   = state_finalize_,
  },
  {
    .state      = 2,
    .name       = "2",
    .initialize = state_initialize_,
    .update     = state_update_,
    .finalize   = state_finalize_,
  },
  {
    .state      = 3,
    .name       = "3",
    .initialize = state3_initialize_,
  },
};

int main(void) {
  statman_state_t state = 0;
  size_t          time  = 0;

  /* state0 -> state1 */
  statman_update(table_, &time, &state);
  assert(state == 1);
  assert(time  == 2);

  /* state1 -> state2 */
  statman_update(table_, &time, &state);
  assert(state == 2);
  assert(time  == 4);

  /* state2 -> state0 */
  statman_update(table_, &time, &state);
  assert(state == 0);
  assert(time  == 6);

  /* state0 -> state1 */
  statman_update(table_, &time, &state);
  assert(state == 1);
  assert(time  == 8);

  /* stat1's finalizer forces to move to 0 */
  statman_transition_to(table_, &time, &state, 2);
  assert(state == 0);
  assert(time  == 8);

  static const check_point_t expects[] = {
    CHECK_POINT_STATE0_UPDATE,
    CHECK_POINT_STATE1_INITIALIZE,
    CHECK_POINT_STATE1_UPDATE,

    CHECK_POINT_STATE1_UPDATE,
    CHECK_POINT_STATE2_INITIALIZE,
    CHECK_POINT_STATE2_UPDATE,

    CHECK_POINT_STATE2_UPDATE,
    CHECK_POINT_STATE3_INITIALIZE,
    CHECK_POINT_STATE0_INITIALIZE,
    CHECK_POINT_STATE0_UPDATE,

    CHECK_POINT_STATE0_UPDATE,
    CHECK_POINT_STATE1_INITIALIZE,
    CHECK_POINT_STATE1_UPDATE,

    CHECK_POINT_STATE1_FINALIZE,
    CHECK_POINT_STATE0_INITIALIZE,
  };

  assert(actual_length_ == sizeof(expects)/sizeof(expects[0]));
  for (size_t i = 0; i < actual_length_; ++i) {
    assert(expects[i] == actual_[i]);
  }
}
