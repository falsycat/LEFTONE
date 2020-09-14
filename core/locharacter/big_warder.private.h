static void locharacter_big_warder_update_passive_action_(
    locharacter_base_t* c) {
  assert(c != NULL);

  static const float beat = LOCHARACTER_BIG_WARDER_BEAT;

  const locharacter_big_warder_param_t* p = (typeof(p)) c->data;
  if (!locharacter_event_holder_has_control(&p->event)) return;

  const uint64_t dt = c->cache.time - c->last_update_time;
  const uint64_t t  = c->cache.time - p->event.start_time;

  const float beats      = t/beat;
  const float last_beats = t > dt? (t-dt)/beat: 0;

# define name_pos_(name, x, y)  \
    locommon_position_t name = c->cache.ground->super.pos;  \
    vec2_addeq(&name.fract, &vec2(x, y));  \
    locommon_position_reduce(&name);

  name_pos_(top,                0, .8f);
  name_pos_(center,             0, .25f);
  name_pos_(left,            -.3f, .2f);
  name_pos_(right,            .3f, .2f);
  name_pos_(leftbottom,      -.4f, .1f);
  name_pos_(rightbottom,      .4f, .1f);
  name_pos_(leftbottom_off,  -.6f, .1f);
  name_pos_(rightbottom_off,  .6f, .1f);

# undef name_pos_

# define trigger_on_(x) (last_beats < (x) && beats >= (x))

  /* ---- intro -> A melody ---- */
  if (trigger_on_(12)) {
    lobullet_base_t* b = lobullet_pool_create(c->bullets);
    lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
          .owner     = c->super.super.id,
          .pos       = center,
          .size      = vec2(.4f, .4f),
          .angle     = MATH_PI/4,
          .color     = vec4(1, 1, 1, .4f),
          .silent    = true,
          .beat      = beat,
          .step      = 4,
          .knockback = .1f,
          .effect    = loeffect_immediate_damage(
              c->recipient.status.attack/4),
        }));
    loentity_store_add(c->entities, &b->super.super);
  }

  /* ---- A melody ---- */
  for (size_t i = 48; i < 80; i+=8) {
    for (size_t j = 0; j < 2; ++j) {
      if (trigger_on_(i-4 + j*4)) {
        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_bomb_triangle_build(b, (&(lobullet_bomb_param_t) {
              .owner     = c->super.super.id,
              .pos       = j? leftbottom: rightbottom,
              .size      = vec2(.05f, .15f),
              .angle     = j? 0: MATH_PI,
              .color     = vec4(1, 1, 1, .6f),
              .silent    = true,
              .beat      = beat,
              .step      = 4,
              .knockback = .1f,
              .effect    = loeffect_immediate_damage(
                  c->recipient.status.attack/2),
            }));
        loentity_store_add(c->entities, &b->super.super);
      }
      if (trigger_on_(i + j*4)) {
        static const float speed = 1.4f;
        static const float accel = .7f / (beat*2);

        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_linear_triangle_build(b, (&(lobullet_linear_param_t) {
              .owner        = c->super.super.id,
              .pos          = j? leftbottom_off: rightbottom_off,
              .size         = vec2(.05f, .15f),
              .velocity     = vec2(j?  speed: -speed, 0),
              .acceleration = vec2(j? -accel:  accel, 0),
              .color        = vec4(1, 1, 1, .8f),
              .duration     = beat*2,
              .knockback    = .1f,
              .effect       = loeffect_immediate_damage(
                  c->recipient.status.attack/2),
            }));
        loentity_store_add(c->entities, &b->super.super);
      }
    }
  }

  /* ---- B melody ---- */
  static const int32_t bmelo_trigger_beats[] = {92, 108};
  static const size_t  bmelo_trigger_counts  =
      sizeof(bmelo_trigger_beats)/sizeof(bmelo_trigger_beats[0]);
  for (size_t i = 0; i < bmelo_trigger_counts; ++i) {
    const int32_t st = bmelo_trigger_beats[i];
    for (int32_t j = 0; j < 4; ++j) {
      if (trigger_on_(st + j/2.f)) {
        for (int32_t x = -2; x <= 2; ++x) {
          locommon_position_t pos = center;
          vec2_addeq(&pos.fract, &vec2(x/2.f*.45f, (j-1)/4.f*.3f));
          locommon_position_reduce(&pos);

          lobullet_base_t* b = lobullet_pool_create(c->bullets);
          lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
                .owner     = c->super.super.id,
                .pos       = pos,
                .size      = vec2(.1f, .1f),
                .angle     = MATH_PI/4,
                .color     = vec4(1, 1, 1, .6f),
                .silent    = true,
                .beat      = beat*2,
                .step      = 2,
                .knockback = .1f,
                .effect    = loeffect_immediate_damage(
                    c->recipient.status.attack/2),
              }));
          loentity_store_add(c->entities, &b->super.super);
        }
      }
    }
  }

  /* ---- C melody ---- */
  for (int32_t i = 0; i < 8; ++i) {
    for (int32_t x = -10; x <= 10; ++x) {
      if (trigger_on_(112 + i*4 + (x+10)/100.f)) {
        locommon_position_t pos = center;
        pos.fract.x += x/10.f*.47f;
        pos.fract.y -= .13f;
        locommon_position_reduce(&pos);

        lobullet_base_t* b = lobullet_pool_create(c->bullets);
        lobullet_bomb_square_build(b, (&(lobullet_bomb_param_t) {
              .owner     = c->super.super.id,
              .pos       = pos,
              .size      = vec2(.06f, .06f),
              .angle     = MATH_PI/4,
              .color     = vec4(1, 1, 1, .4f),
              .silent    = true,
              .beat      = beat,
              .step      = 4,
              .knockback = .1f,
              .effect    = loeffect_immediate_damage(
                  c->recipient.status.attack/2),
            }));
        loentity_store_add(c->entities, &b->super.super);
      }
    }
  }

# undef trigger_on_
}
