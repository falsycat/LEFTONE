#include "./ticker.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <msgpack.h>

#include "util/mpkutil/get.h"

void locommon_ticker_initialize(locommon_ticker_t* ticker, uint64_t time) {
  assert(ticker != NULL);

  *ticker = (typeof(*ticker)) { .time = time, };
}

void locommon_ticker_deinitialize(locommon_ticker_t* ticker) {
  (void) ticker;
}

void locommon_ticker_tick(locommon_ticker_t* ticker, uint64_t time) {
  assert(ticker       != NULL);
  assert(ticker->time <= time);

  ticker->delta = time - ticker->time;
  ticker->delta_f = ticker->delta*1.f / LOCOMMON_TICKER_UNIT;
  ticker->time  = time;
}

void locommon_ticker_pack(
    const locommon_ticker_t* ticker, msgpack_packer* packer) {
  assert(ticker != NULL);
  assert(packer != NULL);

  msgpack_pack_uint64(packer, ticker->time);
}

bool locommon_ticker_unpack(
    locommon_ticker_t* ticker, const msgpack_object* obj) {
  assert(ticker != NULL);

  if (!mpkutil_get_uint64(obj, &ticker->time)) {
    return false;
  }
  return true;
}
