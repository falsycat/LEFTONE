#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <msgpack.h>

#define LOCOMMON_TICKER_UNIT 1000

typedef struct {
  uint64_t time;

  int64_t  delta;
  float    delta_f;
} locommon_ticker_t;

void
locommon_ticker_initialize(
    locommon_ticker_t* ticker,
    uint64_t           time
);

void
locommon_ticker_deinitialize(
    locommon_ticker_t* ticker
);

void
locommon_ticker_tick(
    locommon_ticker_t* ticker,
    uint64_t           time
);

void
locommon_ticker_pack(
    const locommon_ticker_t* ticker,
    msgpack_packer*          packer
);

bool
locommon_ticker_unpack(
    locommon_ticker_t*    ticker,
    const msgpack_object* obj
);
