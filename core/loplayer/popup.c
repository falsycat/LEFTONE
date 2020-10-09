#include "./popup.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "./stance.h"

static loplayer_popup_item_t* loplayer_popup_queue_empty_(
    loplayer_popup_t* popup) {
  assert(popup != NULL);

  const size_t next_tail = (popup->queue_tail+1)%LOPLAYER_POPUP_QUEUE_LENGTH;
  if (next_tail == popup->queue_head) return NULL;

  const size_t i = popup->queue_tail;
  popup->queue_tail = next_tail;
  return &popup->queue[i];
}

void loplayer_popup_initialize(loplayer_popup_t* popup) {
  assert(popup  != NULL);

  *popup = (typeof(*popup)) {0};
}

void loplayer_popup_deinitialize(loplayer_popup_t* popup) {
  assert(popup  != NULL);

}

void loplayer_popup_queue_new_stance(
    loplayer_popup_t* popup, loplayer_stance_t stance) {
  assert(popup  != NULL);

  loplayer_popup_item_t* item = loplayer_popup_queue_empty_(popup);
  if (item == NULL) return;

  *item = (typeof(*item)) {
    .type   = LOPLAYER_POPUP_ITEM_TYPE_NEW_STANCE,
    .stance = stance,
  };
}

const loplayer_popup_item_t* loplayer_popup_enqueue(loplayer_popup_t* popup) {
  assert(popup != NULL);

  if (popup->queue_tail == popup->queue_head) return NULL;

  const size_t i = popup->queue_head;
  popup->queue_head = (popup->queue_head+1)%LOPLAYER_POPUP_QUEUE_LENGTH;
  return &popup->queue[i];
}
