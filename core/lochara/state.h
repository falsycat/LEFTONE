#pragma once

#include <stddef.h>

#include "util/math/vector.h"
#include "util/statman/statman.h"

#include "core/loshader/character.h"

typedef enum {
  /* BENUM BEGIN lochara_state */
  LOCHARA_STATE_STAND,

  LOCHARA_STATE_WALK_LEFT,
  LOCHARA_STATE_WALK_RIGHT,

  LOCHARA_STATE_SPRINT_LEFT,
  LOCHARA_STATE_SPRINT_RIGHT,

  LOCHARA_STATE_DODGE_LEFT,
  LOCHARA_STATE_DODGE_RIGHT,

  LOCHARA_STATE_JUMP,

  LOCHARA_STATE_TELEPORT_BEHIND,
  LOCHARA_STATE_TELEPORT_FRONT,

  LOCHARA_STATE_THRUST_IN,
  LOCHARA_STATE_THRUST_OUT,
  LOCHARA_STATE_SLASH,
  LOCHARA_STATE_SPELL,
  LOCHARA_STATE_SHOOT,

  LOCHARA_STATE_DOWN,
  LOCHARA_STATE_DEAD,
  LOCHARA_STATE_REVIVE,
  LOCHARA_STATE_RESUSCITATE,

  LOCHARA_STATE_GUARD,
  /* BENUM END */
} lochara_state_t;

/* generated benum utils */
#include "core/lochara/benum/state.h"


/* ---- default state handlers ---- */

void
lochara_state_initialize_any_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);
void
lochara_state_cancel_transition_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_state_update_move_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_state_initialize_dodge_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_state_update_dodge_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_state_initialize_jump_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);

void
lochara_state_update_teleport_(
    const statman_meta_t* meta,
    void*                 instance,
    statman_state_t*      next
);


/* ---- state meta constructor ---- */

typedef struct {
  vec2_t acceleration;
  float  speed;
  vec2_t velocity;

  uint64_t                       period;
  loshader_character_motion_id_t motion1;
  loshader_character_motion_id_t motion2;
} lochara_state_move_param_t;
#define lochara_state_stand(...)  \
    (statman_meta_t) {  \
      .state = LOCHARA_STATE_STAND,  \
      .name  = "STAND",  \
      .data  = &(lochara_state_move_param_t) {  \
        .speed = 0,  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_move_,  \
    }
#define lochara_state_walk(...)  \
    (statman_meta_t) {  \
      .state = LOCHARA_STATE_WALK_LEFT,  \
      .name  = "WALK_LEFT",  \
      .data  = &(lochara_state_move_param_t) {  \
        .speed    = 1,  \
        .velocity = {{-1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_move_,  \
    },  \
    (statman_meta_t) {  \
      .state = LOCHARA_STATE_WALK_RIGHT,  \
      .name  = "WALK_RIGHT",  \
      .data  = &(lochara_state_move_param_t) {  \
        .speed    = 1,  \
        .velocity = {{1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_move_,  \
    }
#define lochara_state_sprint(...)  \
    (statman_meta_t) {  \
      .state = LOCHARA_STATE_SPRINT_LEFT,  \
      .name  = "SPRINT_LEFT",  \
      .data  = &(lochara_state_move_param_t) {  \
        .velocity = {{-1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_move_,  \
    },  \
    (statman_meta_t) {  \
      .state = LOCHARA_STATE_SPRINT_RIGHT,  \
      .name  = "SPRINT_RIGHT",  \
      .data  = &(lochara_state_move_param_t) {  \
        .velocity = {{1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_move_,  \
    }

typedef struct {
  uint64_t duration;
  float    speed;
  vec2_t   acceleration;
  vec2_t   velocity;

  loshader_character_motion_id_t motion1;
  loshader_character_motion_id_t motion2;
} lochara_state_dodge_param_t;
#define lochara_state_dodge(...)  \
    (statman_meta_t) {  \
      .state      = LOCHARA_STATE_DODGE_LEFT,  \
      .name       = "DODGE_LEFT",  \
      .data  = &(lochara_state_dodge_param_t) {  \
        .velocity = {{-1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_dodge_,  \
      .update     = lochara_state_update_dodge_,  \
      .finalize   = lochara_state_cancel_transition_,  \
    },  \
    (statman_meta_t) {  \
      .state      = LOCHARA_STATE_DODGE_RIGHT,  \
      .name       = "DODGE_RIGHT",  \
      .data  = &(lochara_state_dodge_param_t) {  \
        .velocity = {{1, 0}},  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_dodge_,  \
      .update     = lochara_state_update_dodge_,  \
      .finalize   = lochara_state_cancel_transition_,  \
    }

#define lochara_state_jump()  \
    (statman_meta_t) {  \
      .state      = LOCHARA_STATE_JUMP,  \
      .name       = "JUMP",  \
      .initialize = lochara_state_initialize_jump_,  \
    }

typedef struct {
  uint64_t duration;
  float    direction;
  vec2_t   offset;

  loshader_character_motion_id_t motion1;
  loshader_character_motion_id_t motion2;
} lochara_state_teleport_param_t;
#define lochara_state_teleport(...)  \
    (statman_meta_t) {  \
      .state  = LOCHARA_STATE_TELEPORT_FRONT,  \
      .name   = "TELEPORT_FRONT",  \
      .data  = &(lochara_state_teleport_param_t) {  \
        .direction = 1,  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_teleport_,  \
    },  \
    (statman_meta_t) {  \
      .state  = LOCHARA_STATE_TELEPORT_BEHIND,  \
      .name   = "TELEPORT_BEHIND",  \
      .data  = &(lochara_state_teleport_param_t) {  \
        .direction = -1,  \
        __VA_ARGS__  \
      },  \
      .initialize = lochara_state_initialize_any_,  \
      .update     = lochara_state_update_teleport_,  \
    }
