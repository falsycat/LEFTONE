#include "./encephalon.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#include <msgpack.h>

#include "util/chaos/xorshift.h"
#include "util/math/algorithm.h"
#include "util/math/vector.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/msgpack.h"
#include "core/loentity/entity.h"
#include "core/loplayer/player.h"
#include "core/loresource/sound.h"
#include "core/loshader/character.h"

#include "./base.h"
#include "./misc.h"

typedef struct {
  float progress;
} locharacter_encephalon_param_t;

_Static_assert(
    sizeof(locharacter_encephalon_param_t) <= LOCHARACTER_BASE_DATA_MAX_SIZE);

static const vec2_t locharacter_encephalon_size_ = vec2(.1f, .1f);

#define LOCHARACTER_ENCEPHALON_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("progress", progress);  \
} while (0)
#define LOCHARACTER_ENCEPHALON_PARAM_TO_PACK_COUNT 1

static void locharacter_encephalon_update_progress_(locharacter_base_t* base) {
  assert(base != NULL);

  locharacter_encephalon_param_t* p = (typeof(p)) base->data;

  const vec2_t* player = &base->cache.player_pos;
  const bool    near   =
      MATH_ABS(player->x) < 1 &&
      0 <= player->y && player->y < locharacter_encephalon_size_.y;

  if (near && base->state == LOCHARACTER_STATE_WAIT) {
    p->progress += base->ticker->delta_f;
    if (p->progress >= 1) {
      loplayer_touch_encephalon(base->player);
      base->state = LOCHARACTER_STATE_COOLDOWN;
      loresource_sound_play(base->res->sound, "touch_gate");
    }
  } else {
    p->progress -= base->ticker->delta_f * 2;
    if (!near) base->state = LOCHARACTER_STATE_WAIT;
  }
  p->progress = MATH_CLAMP(p->progress, 0, 1);
}

bool locharacter_encephalon_update(locharacter_base_t* base) {
  assert(base != NULL);

  locharacter_encephalon_update_progress_(base);

  const locharacter_encephalon_param_t* p = (typeof(p)) base->data;

  base->pos = vec2(0, locharacter_encephalon_size_.y);

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id   = LOSHADER_CHARACTER_ID_ENCEPHALON,
    .from_motion_id = LOSHADER_CHARACTER_MOTION_ID_STAND1,
    .to_motion_id   = LOSHADER_CHARACTER_MOTION_ID_STAND1,
    .color          = vec4(p->progress*.5f, 0, 0, .95f),
    .size           = locharacter_encephalon_size_,
  };
  if (base->state == LOCHARACTER_STATE_COOLDOWN && p->progress > 0) {
    base->cache.instance.color.w *=
        chaos_xorshift(base->ticker->time)%100/100.f;
  }
  return true;
}

void locharacter_encephalon_build(
    locharacter_base_t* base, loentity_id_t ground) {
  assert(base != NULL);

  base->type = LOCHARACTER_TYPE_ENCEPHALON;

  base->ground = ground;
  base->pos    = vec2(0, 0);

  base->state = LOCHARACTER_STATE_WAIT;
}

void locharacter_encephalon_pack_data(
    const locharacter_base_t* base, msgpack_packer* packer) {
  assert(base   != NULL);
  assert(packer != NULL);

  const locharacter_encephalon_param_t* p = (typeof(p)) base->data;

  msgpack_pack_map(packer, LOCHARACTER_ENCEPHALON_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &p->var);  \
  } while (0)

  LOCHARACTER_ENCEPHALON_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool locharacter_encephalon_unpack_data(
    locharacter_base_t* base, const msgpack_object* obj) {
  assert(base != NULL);

  locharacter_encephalon_param_t* p = (typeof(p)) base->data;

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &p->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOCHARACTER_ENCEPHALON_PARAM_TO_PACK_EACH_(unpack_);
  return true;

# undef unpack_

# undef item_
}
