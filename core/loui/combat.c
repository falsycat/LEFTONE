#include "./combat.h"

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/math/vector.h"

#include "core/locommon/easing.h"
#include "core/locommon/ticker.h"
#include "core/loplayer/player.h"
#include "core/loshader/set.h"

#define FADE_SPEED_ 50

#define RING_ATTACK_RANGE_ .6f
#define RING_GUARD_RANGE_  .8f

#define RING_BASE_COLOR_(a)      vec4(0, 0, 0, (a)*.6f)
#define RING_CLOCKHAND_COLOR_(a) vec4(1, 1, 1, (a))
#define RING_ATTACK_COLOR_(a)    vec4(1, 0, 0, (a))
#define RING_GUARD_COLOR_(a)     vec4(0, 0, 1, (a))

#define RING_TAIL_PADDING_ 500

void loui_combat_initialize(
    loui_combat_t*           combat,
    loshader_set_t*          shaders,
    const locommon_ticker_t* ticker,
    const loplayer_t*        player) {
  assert(combat  != NULL);
  assert(shaders != NULL);
  assert(ticker  != NULL);
  assert(player  != NULL);

  *combat = (typeof(*combat)) {
    .shaders = shaders,
    .ticker  = ticker,
    .player  = player,
  };
}

void loui_combat_deinitialize(loui_combat_t* combat) {
  assert(combat != NULL);

}

void loui_combat_update(loui_combat_t* combat) {
  assert(combat != NULL);

  const float dt = combat->ticker->delta_f;

  const loplayer_combat_attack_t* la = combat->player->combat.last_attack;
  locommon_easing_smooth_float(&combat->alpha, !!(la != NULL), dt*FADE_SPEED_);
  if (la == NULL) {
    if (combat->ring_end <= combat->ticker->time) {
      combat->ring_end = 0;
    }
    return;
  }

  if (combat->ring_end <= combat->ring_start) {
    combat->ring_start = combat->ticker->time;
  }
  combat->ring_end = la->start + la->duration + RING_TAIL_PADDING_;
}

void loui_combat_draw(const loui_combat_t* combat) {
  assert(combat != NULL);

  if (combat->alpha <= 0) return;

  loshader_combat_ring_drawer_add_instance(
      &combat->shaders->drawer.combat_ring,
      &(loshader_combat_ring_drawer_instance_t) {
        .range = -1,  /* = base */
        .color = RING_BASE_COLOR_(combat->alpha),
      });

  const uint64_t ring_st = combat->ring_start;
  const uint64_t ring_ed = combat->ring_end;
  if (ring_ed < ring_st) return;

  const uint64_t t   = combat->ticker->time;
  const uint64_t dur = ring_ed - ring_st;
  loshader_combat_ring_drawer_add_instance(
      &combat->shaders->drawer.combat_ring,
      &(loshader_combat_ring_drawer_instance_t) {
        .range = 0,  /* = clockhand */
        .start = (t - ring_st)*1.f/dur,
        .color = RING_CLOCKHAND_COLOR_(combat->alpha),
      });

  const loplayer_combat_t* pcombat = &combat->player->combat;
  for (size_t i = 0; i < LOPLAYER_COMBAT_RESERVE; ++i) {
    const loplayer_combat_attack_t* a = &pcombat->attacks[i];
    if (a->duration == 0) continue;

    const uint64_t st = MATH_MAX(ring_st, a->start);
    const uint64_t ed = a->start+a->duration;

    loshader_combat_ring_drawer_add_instance(
        &combat->shaders->drawer.combat_ring,
        &(loshader_combat_ring_drawer_instance_t) {
          .range = RING_ATTACK_RANGE_,
          .start = (st - ring_st)*1.f/dur,
          .end   = (ed - ring_st)*1.f/dur,
          .color = RING_ATTACK_COLOR_(combat->alpha),
        });
  }

  const uint64_t gst = MATH_MAX(ring_st, pcombat->last_guard_start);
  const uint64_t ged =
      pcombat->last_guard_start <= pcombat->last_guard_end?
      pcombat->last_guard_end: t;
  if (ring_st <= gst && gst < ged && ged < ring_ed) {
    loshader_combat_ring_drawer_add_instance(
        &combat->shaders->drawer.combat_ring,
        &(loshader_combat_ring_drawer_instance_t) {
          .range = RING_GUARD_RANGE_,
          .start = (gst - ring_st)*1.f/dur,
          .end   = (ged - ring_st)*1.f/dur,
          .color = RING_GUARD_COLOR_(combat->alpha),
        });
  }
}
