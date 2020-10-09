#pragma once

#include <stddef.h>
#include <stdint.h>

#include "./stance.h"

typedef enum {
  LOPLAYER_POPUP_ITEM_TYPE_NEW_STANCE,
} loplayer_popup_item_type_t;

typedef struct {
  loplayer_popup_item_type_t type;
  union {
    loplayer_stance_t stance;
  };
} loplayer_popup_item_t;

typedef struct {
# define LOPLAYER_POPUP_QUEUE_LENGTH 32
  loplayer_popup_item_t queue[LOPLAYER_POPUP_QUEUE_LENGTH];

  size_t queue_tail;
  size_t queue_head;

  uint64_t last_enqueue;
} loplayer_popup_t;

void
loplayer_popup_initialize(
    loplayer_popup_t* popup
);

void
loplayer_popup_deinitialize(
    loplayer_popup_t* popup
);

void
loplayer_popup_queue_new_stance(
    loplayer_popup_t* popup,
    loplayer_stance_t stance
);

const loplayer_popup_item_t*
loplayer_popup_enqueue(
    loplayer_popup_t* popup
);
