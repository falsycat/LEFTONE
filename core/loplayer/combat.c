#include "./combat.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "util/math/algorithm.h"
#include "util/memory/memory.h"
#include "util/mpkutil/get.h"
#include "util/mpkutil/pack.h"

#include "core/locommon/easing.h"
#include "core/locommon/msgpack.h"
#include "core/locommon/ticker.h"
#include "core/loeffect/effect.h"
#include "core/loentity/character.h"
#include "core/loentity/store.h"
#include "core/loresource/sound.h"
#include "core/loshader/combat_ring.h"

#include "./entity.h"
#include "./status.h"

struct loplayer_combat_t {
  /* injected deps */
  loresource_sound_t*            sound;
  const locommon_ticker_t*       ticker;
  loshader_combat_ring_drawer_t* drawer;
  loentity_store_t*              entities;

  loplayer_status_t* status;
  loplayer_entity_t* entity;

  /* temporary cache for drawing */
  uint64_t ring_start;
  uint64_t ring_end;

  uint64_t guard_start;
  uint64_t guard_end;

  uint64_t attack_end;
  float    alpha;

  /* params */
  bool accepted;

  size_t length;
  loplayer_combat_attack_t attacks[1];
};

#define LOPLAYER_COMBAT_GUARD_ERROR_THRESHOLD 0.55f

#define LOPLAYER_COMBAT_ATTACK_PARAM_TO_PACK_EACH_(PROC) do {  \
  PROC("attacker",  attacker);  \
  PROC("start",     start);  \
  PROC("duration",  duration);  \
  PROC("knockback", knockback);  \
  PROC("effect",    effect);  \
} while (0)
#define LOPLAYER_COMBAT_ATTACK_PARAM_TO_PACK_COUNT 5

static bool loplayer_combat_find_attack_index_in_period_(
    loplayer_combat_t* combat, size_t* index, uint64_t st, uint64_t ed) {
  assert(combat != NULL);
  assert(st <= ed);

  static size_t dummy_;
  if (index == NULL) index = &dummy_;

  for (size_t i = 0; i < combat->length; ++i) {
    const uint64_t ist = combat->attacks[i].start;
    const uint64_t ied = combat->attacks[i].duration + ist;

    if (ist < ed && ied > st) {
      *index = i;
      return true;
    }
  }
  return false;
}

static bool loplayer_combat_find_unused_attack_index_(
    loplayer_combat_t* combat, size_t *index) {
  assert(combat != NULL);

  for (size_t i = 0; i < combat->length; ++i) {
    const uint64_t ed = combat->attacks[i].start + combat->attacks[i].duration;
    if (ed <= combat->ticker->time) {
      *index = i;
      return true;
    }
  }
  return false;
}

static void loplayer_combat_execute_reflective_attack_(
    loplayer_combat_t*        combat,
    loplayer_combat_attack_t* attack,
    loentity_character_t*     chara) {
  assert(combat != NULL);
  assert(attack != NULL);

  if (chara == NULL) return;

  const loeffect_t dmg =
      loeffect_immediate_damage(combat->status->recipient.status.attack);
  loentity_character_apply_effect(chara, &dmg);

  vec2_t knockback = attack->knockback;
  vec2_muleq(&knockback, -1);
  loentity_character_knockback(chara, &knockback);

  loresource_sound_play(combat->sound, "reflection");
}

static void loplayer_combat_execute_enemy_attack_(
    loplayer_combat_t*        combat,
    loplayer_combat_attack_t* attack,
    loentity_character_t*     chara) {
  assert(combat != NULL);
  assert(attack != NULL);

  /* chara can be NULL */
  (void) chara;

  loplayer_status_apply_effect(combat->status, &attack->effect);
  loentity_character_knockback(&combat->entity->super, &attack->knockback);
}

static void loplayer_combat_handle_attack_(
    loplayer_combat_t*        combat,
    loplayer_combat_attack_t* attack,
    loentity_character_t*     chara) {
  assert(combat != NULL);
  assert(attack != NULL);

  if (combat->guard_start < combat->guard_end) {
    const uint64_t atked = attack->start + attack->duration;

    const uint64_t stdiff =
        MATH_MAX(attack->start, combat->guard_start) -
        MATH_MIN(attack->start, combat->guard_start);
    const uint64_t eddiff =
        MATH_MAX(atked, combat->guard_end) -
        MATH_MIN(atked, combat->guard_end);

    const float guard_error = (stdiff + eddiff)*1.0f / attack->duration;

    const bool back_attack =
        attack->knockback.x * combat->entity->direction > 0;

    float guard_error_thresh = LOPLAYER_COMBAT_GUARD_ERROR_THRESHOLD;
    if (back_attack) guard_error_thresh /= 10;

    if (guard_error < guard_error_thresh) {
      loplayer_combat_execute_reflective_attack_(combat, attack, chara);
      return;
    }
  }
  loplayer_combat_execute_enemy_attack_(combat, attack, chara);
}

static void loplayer_combat_draw_ring_base_(
    const loplayer_combat_t* combat) {
  assert(combat != NULL);

  loshader_combat_ring_drawer_add_instance(
      combat->drawer, &(loshader_combat_ring_drawer_instance_t) {
        .range = -1,  /* = draw ring base */
        .color = vec4(0, 0, 0, .8f*combat->alpha),
      });
}

static void loplayer_combat_draw_attacks_(const loplayer_combat_t* combat) {
  assert(combat != NULL);

  const uint64_t ring_st = combat->ring_start;
  const uint64_t ring_ed = combat->ring_end;
  assert(ring_st <= ring_ed);

  const uint64_t ring_dur = ring_ed - ring_st;

  for (size_t i = 0; i < combat->length; ++i) {
    const uint64_t st = combat->attacks[i].start;
    const uint64_t ed = combat->attacks[i].duration + st;

    if (st < ring_ed && ring_st < ed) {
      const uint64_t rst = st - MATH_MIN(ring_st, st);
      const uint64_t red = ed - ring_st;
      assert(rst <= red);

      loshader_combat_ring_drawer_add_instance(
          combat->drawer, &(loshader_combat_ring_drawer_instance_t) {
            .range = .8f,
            .start = rst*1.f/ring_dur,
            .end   = MATH_MIN(red, ring_dur)*1.f/ring_dur,
            .color = vec4(.7f, .1f, .1f, combat->alpha),
          });
    }
  }
}

static void loplayer_combat_draw_guard_(const loplayer_combat_t* combat) {
  assert(combat != NULL);

  const bool now_guarding = (combat->guard_start > combat->guard_end);
  if (!now_guarding && combat->guard_end <= combat->ring_start) {
    return;
  }

  const float ring_dur = combat->ring_end - combat->ring_start;
  assert(ring_dur > 0);

  const uint64_t st = combat->guard_start -
      MATH_MIN(combat->ring_start, combat->guard_start);
  const uint64_t ed =
      (now_guarding? combat->ticker->time: combat->guard_end) -
      combat->ring_start;

  loshader_combat_ring_drawer_add_instance(
      combat->drawer, &(loshader_combat_ring_drawer_instance_t) {
        .range = .7f,
        .start = st/ring_dur,
        .end   = ed/ring_dur,
        .color = vec4(.1f, .1f, .7f, combat->alpha),
      });
}

static void loplayer_combat_draw_clockhand_(
    const loplayer_combat_t* combat) {
  assert(combat != NULL);

  const uint64_t ring_dur = combat->ring_end - combat->ring_start;
  assert(ring_dur > 0);

  assert(combat->ticker->time >= combat->ring_start);
  const uint64_t cur  = combat->ticker->time - combat->ring_start;
  const float    curf = cur*1.f/ring_dur;

  loshader_combat_ring_drawer_add_instance(
      combat->drawer, &(loshader_combat_ring_drawer_instance_t) {
        .range = 0,  /* = draw clockhand */
        .start = curf,
        .color = vec4(1, 1, 1, combat->alpha),
      });
}

loplayer_combat_t* loplayer_combat_new(
    loresource_sound_t*            sound,
    loshader_combat_ring_drawer_t* drawer,
    const locommon_ticker_t*       ticker,
    loentity_store_t*              entities,
    loplayer_status_t*             status,
    loplayer_entity_t*             entity,
    size_t                         length) {
  assert(sound    != NULL);
  assert(drawer   != NULL);
  assert(ticker   != NULL);
  assert(entities != NULL);
  assert(status   != NULL);
  assert(entity   != NULL);
  assert(length > 0);

  loplayer_combat_t* combat = memory_new(
      sizeof(*combat) + (length-1)*sizeof(combat->attacks[0]));
  *combat = (typeof(*combat)) {
    .sound    = sound,
    .drawer   = drawer,
    .ticker   = ticker,
    .entities = entities,
    .status   = status,
    .entity   = entity,
    .length   = length,
  };

  for (size_t i = 0; i < combat->length; ++i) {
    combat->attacks[i] = (typeof(combat->attacks[0])) {0};
  }
  return combat;
}

void loplayer_combat_delete(loplayer_combat_t* combat) {
  if (combat == NULL) return;

  memory_delete(combat);
}

bool loplayer_combat_add_attack(
    loplayer_combat_t* combat, const loplayer_combat_attack_t* attack) {
  assert(combat != NULL);
  assert(attack != NULL);

  if (loplayer_combat_find_attack_index_in_period_(
      combat, NULL, attack->start, attack->start + attack->duration)) {
    return false;
  }

  size_t index;
  if (!loplayer_combat_find_unused_attack_index_(combat, &index)) {
    return false;
  }

  combat->attacks[index] = *attack;
  return true;
}

bool loplayer_combat_accept_all_attacks(loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (combat->accepted) return true;

  if (!loplayer_combat_find_attack_index_in_period_(
        combat, NULL, combat->ticker->time, SIZE_MAX)) {
    return false;
  }

  combat->accepted = true;
  return true;
}

void loplayer_combat_drop_all_attacks(loplayer_combat_t* combat) {
  assert(combat != NULL);

  for (size_t i = 0; i < combat->length; ++i) {
    combat->attacks[i] = (typeof(combat->attacks[0])) {0};
  }
  combat->accepted    = false;
  combat->ring_start  = 0;
  combat->ring_end    = 0;
  combat->guard_start = 0;
  combat->guard_end   = 0;
  combat->attack_end  = 0;
}

void loplayer_combat_guard(loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (combat->ring_end <= combat->ticker->time ||
      combat->guard_start > combat->guard_end) {
    return;
  }
  combat->guard_start = combat->ticker->time;

  loresource_sound_play(combat->sound, "guard");
}

void loplayer_combat_unguard(loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (combat->guard_start <= combat->guard_end) return;

  combat->guard_end = combat->ticker->time;
}

void loplayer_combat_update(loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (!combat->accepted) return;

  const uint64_t cur = combat->ticker->time;
  const uint64_t pre = cur - combat->ticker->delta;

  combat->ring_end = 0;
  for (size_t i = 0; i < combat->length; ++i) {
    loentity_store_iterator_t itr = (typeof(itr)) {0};

    if (!loentity_store_find_item_by_id(
        combat->entities, &itr, combat->attacks[i].attacker)) {
      combat->attacks[i].start    = 0;
      combat->attacks[i].duration = 0;
      continue;
    }

    const uint64_t st = combat->attacks[i].start;
    const uint64_t ed = combat->attacks[i].duration + st;

    const bool pre_active = st <= pre && pre < ed;
    const bool cur_active = st <= cur && cur < ed;
    if (!pre_active && cur_active) {
      combat->attack_end = ed;
    } else if (pre_active && !cur_active) {
      loplayer_combat_handle_attack_(
          combat, &combat->attacks[i], itr.character);
    }
    combat->ring_end = MATH_MAX(combat->ring_end, ed);
  }
  if (combat->ring_end > cur) {
    if (combat->ring_start == 0) {
      combat->ring_start = cur;
    }
    locommon_easing_smooth_float(
        &combat->alpha, 1, combat->ticker->delta_f*10);
  } else {
    combat->alpha = 0;
    loplayer_combat_drop_all_attacks(combat);
  }
}

void loplayer_combat_draw_ui(const loplayer_combat_t* combat) {
  assert(combat != NULL);

  if (!combat->accepted || combat->ring_end <= combat->ticker->time) {
    return;
  }
  loplayer_combat_draw_ring_base_(combat);
  loplayer_combat_draw_attacks_(combat);
  loplayer_combat_draw_guard_(combat);
  loplayer_combat_draw_clockhand_(combat);
}

void loplayer_combat_pack(
    const loplayer_combat_t* combat, msgpack_packer* packer) {
  assert(combat != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, 1);

  mpkutil_pack_str(packer, "attacks");

  size_t len = 0;
  for (size_t i = 0; i < combat->length; ++i) {
    const uint64_t st  = combat->attacks[i].start;
    const uint64_t ed  = combat->attacks[i].duration + st;
    const uint64_t cur = combat->ticker->time;

    if (st <= cur && cur < ed) ++len;
  }
  msgpack_pack_array(packer, len);

  for (size_t i = 0; i < combat->length; ++i) {
    const uint64_t st  = combat->attacks[i].start;
    const uint64_t ed  = combat->attacks[i].duration + st;
    const uint64_t cur = combat->ticker->time;

    if (st <= cur && cur < ed) {
      loplayer_combat_attack_pack(&combat->attacks[i], packer);
    }
  }
}

bool loplayer_combat_unpack(
    loplayer_combat_t* combat, const msgpack_object* obj) {
  assert(combat != NULL);

  loplayer_combat_drop_all_attacks(combat);

  if (obj == NULL) return false;

  const msgpack_object_map* root = mpkutil_get_map(obj);

  const msgpack_object_array* array =
      mpkutil_get_array(mpkutil_get_map_item_by_str(root, "attacks"));

  size_t src = 0, dst = 0;
  while (src < array->size && dst < combat->length) {
    if (loplayer_combat_attack_unpack(
          &combat->attacks[dst], &array->ptr[src++])) {
      ++dst;
    }
  }
  return true;
}

void loplayer_combat_attack_pack(
    const loplayer_combat_attack_t* attack,
    msgpack_packer*                 packer) {
  assert(attack != NULL);
  assert(packer != NULL);

  msgpack_pack_map(packer, LOPLAYER_COMBAT_ATTACK_PARAM_TO_PACK_COUNT);

# define pack_(name, var) do {  \
    mpkutil_pack_str(packer, name);  \
    LOCOMMON_MSGPACK_PACK_ANY(packer, &attack->var);  \
  } while (0)

  LOPLAYER_COMBAT_ATTACK_PARAM_TO_PACK_EACH_(pack_);

# undef pack_
}

bool loplayer_combat_attack_unpack(
    loplayer_combat_attack_t* attack,
    const msgpack_object*     obj) {
  assert(attack != NULL);

  const msgpack_object_map* root = mpkutil_get_map(obj);

# define item_(v) mpkutil_get_map_item_by_str(root, v)

# define unpack_(name, var) do {  \
    if (!LOCOMMON_MSGPACK_UNPACK_ANY(item_(name), &attack->var)) {  \
      return false;  \
    }  \
  } while (0)

  LOPLAYER_COMBAT_ATTACK_PARAM_TO_PACK_EACH_(unpack_);
  return true;

# undef unpack_
# undef item_
}
