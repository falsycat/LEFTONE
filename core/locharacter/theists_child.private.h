static void locharacter_theists_child_update_passive_action_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_THEISTS_CHILD_BEAT;

  const locharacter_theists_child_param_t* p = (typeof(p)) c->data;
  if (!locharacter_event_holder_has_control(&p->event)) return;

  const uint64_t dt = c->cache.time - c->last_update_time;
  const uint64_t t  = c->cache.time - p->event.start_time;

  const float beats      = t/beat;
  const float last_beats = t > dt? (t-dt)/beat: 0;

# define name_pos_(name, x, y)  \
    locommon_position_t name = c->cache.ground->super.pos;  \
    vec2_addeq(&name.fract, &vec2(x, y));  \
    locommon_position_reduce(&name);

  name_pos_(top,         0, .8f);
  name_pos_(lefttop, -.25f, .8f);
  name_pos_(righttop, .25f, .8f);
  name_pos_(center,      0, .25f);
  name_pos_(left,     -.3f, .2f);
  name_pos_(right,     .3f, .2f);

# undef name_pos_

# define trigger_on_(x) (last_beats < (x) && beats >= (x))

  /* ---- intro -> A melody ---- */
  if (trigger_on_(56)) {
    for (size_t i = 0; i < 2; ++i) {
        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_bomb_triangle_build(b, (&(lobullet_bomb_param_t) {
              .owner     = c->super.super.id,
              .pos       = i? left: right,
              .size      = vec2(.05f, .15f),
              .angle     = -MATH_PI/2,
              .color     = vec4(1, 1, 1, .8f),
              .silent    = true,
              .beat      = beat,
              .step      = 8,
              .knockback = .1f,
              .effect    = loeffect_immediate_damage(
                  c->recipient.status.attack/2),
            }));
        loentity_store_add(c->entities, &b->super.super);
    }
  }
  if (trigger_on_(64)) {
    for (size_t i = 0; i < 2; ++i) {
        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_linear_triangle_build(b, (&(lobullet_linear_param_t) {
              .owner        = c->super.super.id,
              .pos          = i? lefttop: righttop,
              .size         = vec2(.05f, .15f),
              .velocity     = vec2(0, -1.4f/(beat/1000*2)),
              .acceleration = vec2(0, 1/(beat/1000*2)),
              .color        = vec4(1, 1, 1, .8f),
              .duration     = beat*2,
              .knockback    = .1f,
              .effect       = loeffect_immediate_damage(
                  c->recipient.status.attack),
            }));
        loentity_store_add(c->entities, &b->super.super);
    }
  }

  /* ---- B melody ---- */
  for (size_t i = 128, cnt = 0; i < 192; i+=4, ++cnt) {
    if (trigger_on_(i)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .pos       = cnt%2 == 0? left: right,
            .size      = vec2(.13f, .13f),
            .angle     = MATH_PI/4,
            .color     = vec4(1, 1, 1, .8f),
            .silent    = true,
            .beat      = LOCHARACTER_THEISTS_CHILD_BEAT,
            .step      = 4,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(
                c->recipient.status.attack/2),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }
  for (size_t i = 128; i < 192; i+=4) {
    if (trigger_on_(i)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_linear_triangle_build(b, (&(lobullet_linear_param_t) {
            .owner        = c->super.super.id,
            .pos          = top,
            .size         = vec2(.05f, .2f),
            .velocity     = vec2(0, -1.4f/(beat/1000*2)),
            .acceleration = vec2(0, 1/(beat/1000*2)),
            .color        = vec4(1, 1, 1, .8f),
            .duration     = beat*2,
            .knockback    = .1f,
            .effect       = loeffect_immediate_damage(
                c->recipient.status.attack),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }

  /* ---- fill-in ---- */
  if (trigger_on_(192)) {
    lobullet_base_t* b = lobullet_pool_create(c->bullets);
    lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
          .owner     = c->super.super.id,
          .pos       = center,
          .size      = vec2(.2f, .2f),
          .angle     = MATH_PI/4,
          .color     = vec4(1, 1, .4f, .8f),
          .silent    = true,
          .beat      = LOCHARACTER_THEISTS_CHILD_BEAT,
          .step      = 8,
          .knockback = .1f,
          .effect    = loeffect_amnesia(
              c->ticker->time + (uint64_t) (8*beat), beat*4),
        }));
    loentity_store_add(c->entities, &b->super.super);
  }

  /* ---- C melody ---- */
  for (size_t i = 200, cnt = 0; i < 232; i+=2, ++cnt) {
    if (trigger_on_(i)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .size      = vec2(.16f, .16f),
            .pos       = cnt%2 == 0? left: right,
            .angle     = MATH_PI/4,
            .color     = vec4(1, 1, 1, .8f),
            .silent    = true,
            .beat      = LOCHARACTER_THEISTS_CHILD_BEAT,
            .step      = 2,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(c->recipient.status.attack),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }

# undef trigger_on_
}
