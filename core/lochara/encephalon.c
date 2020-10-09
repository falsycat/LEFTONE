#include "./encephalon.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "util/chaos/xorshift.h"
#include "util/math/vector.h"

#include "core/locommon/position.h"
#include "core/loeffect/effect.h"
#include "core/loeffect/recipient.h"
#include "core/loentity/ground.h"
#include "core/loshader/character.h"

#include "./base.h"

#define WIDTH_  .1f
#define HEIGHT_ .1f
#define RANGE_  .15f
#define COLOR_  vec4(0, 0, 0, .8f)

bool lochara_encephalon_update(lochara_base_t* base) {
  assert(base != NULL);

  base->cache.instance = (loshader_character_drawer_instance_t) {
    .character_id  = LOSHADER_CHARACTER_ID_ENCEPHALON,
    .size          = vec2(WIDTH_, HEIGHT_),
    .color         = COLOR_,
  };

  vec2_t disp;
  locommon_position_sub(
      &disp, &base->player->entity->super.super.pos, &base->super.super.pos);

  const bool active = fabsf(disp.x) < RANGE_ && fabsf(disp.y) < HEIGHT_;
  if (active) {
    if (base->param.state != LOCHARA_STATE_SPELL) {
      loresource_sound_set_play(
          &base->res->sound, LORESOURCE_SOUND_ID_TOUCH_GATE);
      base->param.last_state_changed = base->ticker->time;
    }
    base->param.state = LOCHARA_STATE_SPELL;

    loentity_character_apply_effect(
        &base->player->entity->super, &loeffect_resuscitate());
    base->player->entity->param.anchor.pos =
        base->player->entity->super.super.pos;

    if (base->param.last_state_changed+500 > base->ticker->time) {
      base->cache.instance.color.w = chaos_xorshift(base->ticker->time)%3 >= 1;
    }
  } else {
    base->param.state = LOCHARA_STATE_STAND;
  }

  if (base->cache.ground == NULL) return false;
  base->super.super.pos = base->cache.ground->super.pos;
  base->super.super.pos.fract.y += base->cache.ground->size.y + HEIGHT_;
  locommon_position_reduce(&base->super.super.pos);

  return true;
}

void lochara_encephalon_build(
    lochara_base_t* base, const loentity_ground_t* gnd) {
  assert(base != NULL);
  assert(gnd  != NULL);

  base->super.super.pos = gnd->super.pos;
  base->super.super.pos.fract.y += gnd->size.y + HEIGHT_;
  locommon_position_reduce(&base->super.super.pos);

  base->param = (typeof(base->param)) {
    .type               = LOCHARA_TYPE_ENCEPHALON,
    .state              = LOCHARA_STATE_STAND,
    .last_state_changed = base->ticker->time,
    .ground             = gnd->super.id,
  };
  loeffect_recipient_initialize(
      &base->param.recipient,
      base->ticker,
      &(loeffect_recipient_status_t) {0});
}
