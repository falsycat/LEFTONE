static void locharacter_greedy_scientist_trigger_chained_mines_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_GREEDY_SCIENTIST_BEAT;

  locommon_position_t center = c->cache.ground->super.pos;
  center.fract.y += .1f;
  locommon_position_reduce(&center);

  lobullet_base_t* b = lobullet_pool_create(c->bullets);
  lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
        .owner     = c->super.super.id,
        .pos       = center,
        .size      = vec2(.07f, .07f),
        .angle     = 0,
        .color     = vec4(1, .9f, .9f, .8f),
        .beat      = beat,
        .step      = 2,
        .knockback = .1f,
        .effect    = loeffect_immediate_damage(
            c->recipient.status.attack/2),
      }));
  loentity_store_add(c->entities, &b->super.super);

  for (int32_t i = -6; i <= 6; ++i) {
    locommon_position_t pos = center;
    pos.fract.x += i/6.f*.5f;
    locommon_position_reduce(&pos);

    lobullet_base_t* b = lobullet_pool_create(c->bullets);
    lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
          .owner     = c->super.super.id,
          .pos       = pos,
          .size      = vec2(.05f, .05f),
          .angle     = 0,
          .color     = vec4(1, 1, 1, .4f),
          .silent    = true,
          .beat      = beat*1.5f,
          .step      = 2,
          .knockback = .1f,
          .effect    = loeffect_immediate_damage(
              c->recipient.status.attack/2),
        }));
    loentity_store_add(c->entities, &b->super.super);
  }
}

static void locharacter_greedy_scientist_shoot_amnesia_bullet_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_GREEDY_SCIENTIST_BEAT;

  locommon_position_t pos = c->super.super.pos;
  pos.fract.y += locharacter_greedy_scientist_size_.y*1.5f;
  locommon_position_reduce(&pos);

  lobullet_base_t* b = lobullet_pool_create(c->bullets);
  lobullet_linear_light_build(b, (&(lobullet_linear_param_t) {
        .owner        = c->super.super.id,
        .pos          = pos,
        .size         = vec2(.06f, .06f),
        .velocity     = vec2(0, .1f),
        .color        = vec4(.8f, .8f, .6f, .8f),
        .acceleration = vec2(0, -2),
        .duration     = 1000,
        .effect       = loeffect_amnesia(c->ticker->time, beat*8),
      }));
  loentity_store_add(c->entities, &b->super.super);
}

static void locharacter_greedy_scientist_update_passive_action_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_GREEDY_SCIENTIST_BEAT;

  const locharacter_greedy_scientist_param_t* p = (typeof(p)) c->data;
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
  name_pos_(lefttop,  -.3f, .8f);
  name_pos_(righttop,  .3f, .8f);
  name_pos_(center,      0, .4f);
  name_pos_(left,     -.3f, .2f);
  name_pos_(right,     .3f, .2f);

# undef name_pos_

# define trigger_on_(x) (last_beats < (x) && beats >= (x))

  /* ---- intro -> A melody ---- */
  for (size_t i = 0; i < 2; ++i) {
    if (trigger_on_(16)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_triangle_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .pos       = i? left: right,
            .size      = vec2(.1f*cos(MATH_PI/6), .1f),
            .angle     = -MATH_PI/2,
            .color     = vec4(1, 1, 1, .4f),
            .silent    = true,
            .beat      = beat*2,
            .step      = 1,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(
                c->recipient.status.attack/4),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
    if (trigger_on_(16.5f)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_triangle_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .pos       = i? left: right,
            .size      = vec2(.1f*cos(MATH_PI/6), .1f),
            .angle     = MATH_PI/2,
            .color     = vec4(1, 1, 1, .4f),
            .silent    = true,
            .beat      = beat*2,
            .step      = 1,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(
                c->recipient.status.attack/4),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
    if (trigger_on_(17)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .pos       = i? left: right,
            .size      = vec2(.12f, .12f),
            .angle     = MATH_PI/4,
            .color     = vec4(1, 1, 1, .4f),
            .silent    = true,
            .beat      = beat*2,
            .step      = 1,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(
                c->recipient.status.attack/4),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }

  for (size_t i = 0; i < 4; ++i) {
    if (trigger_on_(18 + i*.5f)) {
      locommon_position_t pos = center;
      pos.fract.y -= .1f * i;
      locommon_position_reduce(&pos);

      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_bomb_triangle_build(b, (&(lobullet_bomb_param_t) {
            .owner     = c->super.super.id,
            .pos       = pos,
            .size      = vec2(.05f, .2f),
            .angle     = -MATH_PI/2,
            .color     = vec4(1, 1, 1, .4f),
            .silent    = true,
            .beat      = beat,
            .step      = 1,
            .knockback = .1f,
            .effect    = loeffect_immediate_damage(
                c->recipient.status.attack/2),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }

  /* ---- B melody ---- */
  for (size_t i = 52, cnt = 0; i < 84; i+=4, ++cnt) {
    if (trigger_on_(i)) {
      lobullet_base_t* b = lobullet_pool_create(c->bullets);
      lobullet_linear_triangle_build(b, (&(lobullet_linear_param_t) {
            .owner        = c->super.super.id,
            .pos          = cnt%2? lefttop: righttop,
            .size         = vec2(.1f, .3f),
            .velocity     = vec2(0, -1/(beat*4)*1000),
            .color        = vec4(1, 1, 1, .8f),
            .duration     = beat*4,
            .knockback    = .1f,
            .effect       = loeffect_immediate_damage(
                c->recipient.status.attack),
          }));
      loentity_store_add(c->entities, &b->super.super);
    }
  }

  /* ---- C melody ---- */
  for (size_t i = 84, cnt = 0; i < 156; i+=8, ++cnt) {
    if (trigger_on_(i)) {
      for (int32_t x = -1-cnt%2; x <= 2; x+=2) {
        locommon_position_t pos = top;
        pos.fract.x += .18f*x;
        locommon_position_reduce(&pos);

        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_linear_triangle_build(b, (&(lobullet_linear_param_t) {
              .owner        = c->super.super.id,
              .pos          = pos,
              .size         = vec2(.05f, .1f),
              .velocity     = vec2(0, -1/(beat*4)*1000),
              .color        = vec4(1, 1, 1, .8f),
              .duration     = beat*4,
              .knockback    = .1f,
              .effect       = loeffect_immediate_damage(
                  c->recipient.status.attack),
            }));
        loentity_store_add(c->entities, &b->super.super);
      }
    }
  }

# undef trigger_on_
}
