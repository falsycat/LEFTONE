#include "./util.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/rational.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loresource/music.h"
#include "core/loplayer/event.h"

#include "./base.h"

static void locharacter_event_holder_handle_control_lost_(
    locharacter_event_holder_t* holder) {
  assert(holder != NULL);

  if (holder->music != NULL) {
    jukebox_amp_change_volume(&holder->music->amp, 0, &rational(1, 1));
    jukebox_decoder_stop_after(holder->music->decoder, &rational(1, 1));
  }

  holder->param      = NULL;
  holder->start_time = 0;
}

void locharacter_event_holder_initialize(
    locharacter_event_holder_t* holder,
    loresource_music_player_t*  music,
    locharacter_base_t*         owner,
    uint64_t                    duration,
    uint64_t                    start_time) {
  assert(holder != NULL);
  assert(music  != NULL);
  assert(owner  != NULL);
  assert(duration > 0);

  *holder = (typeof(*holder)) {
    .music      = music,
    .owner      = owner,
    .duration   = duration,
    .start_time = start_time,
  };
}

void locharacter_event_holder_deinitialize(
    locharacter_event_holder_t* holder) {
  assert(holder != NULL);

  locharacter_event_holder_release_control(holder);
}

bool locharacter_event_holder_take_control(
    locharacter_event_holder_t* holder) {
  assert(holder                      != NULL);
  assert(holder->owner               != NULL);
  assert(holder->owner->cache.ground != NULL);

  const locharacter_base_t* owner = holder->owner;

  const bool recover = holder->start_time > 0;

  const uint64_t t = recover? owner->cache.time - holder->start_time: 0;
  if (recover && t >= holder->duration) return false;

  holder->param = loplayer_event_take_control(
      owner->player->event, owner->super.super.id);
  if (holder->param == NULL) return false;

  loplayer_event_param_t* p = holder->param;

  p->area_pos = owner->cache.ground->super.pos;
  p->area_pos.fract.y += .4f;
  locommon_position_reduce(&p->area_pos);

  p->area_size = vec2(.45f, .45f);
  p->music     = holder->music;

  if (!recover) {
    loentity_character_apply_effect(
        &owner->player->entity.super,
        &loeffect_curse(owner->ticker->time, holder->duration));
    holder->start_time = owner->cache.time;
  }
  if (holder->music != NULL) {
    jukebox_decoder_play(holder->music->decoder, &rational(t, 1000), false);
    jukebox_amp_change_volume(&holder->music->amp, .8f, &rational(1, 1));
  }
  return true;
}

void locharacter_event_holder_release_control(
    locharacter_event_holder_t* holder) {
  assert(holder != NULL);

  if (holder->param == NULL) return;

  loplayer_event_param_release_control(holder->param);
  locharacter_event_holder_handle_control_lost_(holder);
}

bool locharacter_event_holder_update(locharacter_event_holder_t* holder) {
  assert(holder != NULL);

  if (holder->start_time > holder->owner->ticker->time) {
    holder->start_time = 0;
  }

  loplayer_event_param_t* p = holder->param;
  if (p == NULL) {
    if (holder->start_time > 0) {
      return locharacter_event_holder_take_control(holder);
    }
    return true;
  }

  if (!p->controlled || p->controlled_by != holder->owner->super.super.id) {
    locharacter_event_holder_handle_control_lost_(holder);
    return false;
  }

  if (holder->music != NULL) {
    rational_t r;
    jukebox_decoder_get_seek_position(holder->music->decoder, &r);
    rational_normalize(&r, 1000);
    holder->owner->cache.time = r.num + holder->start_time;
  }
  return true;
}

bool locharacter_event_holder_has_control(
    const locharacter_event_holder_t* holder) {
  assert(holder != NULL);

  return holder->param != NULL;
}
