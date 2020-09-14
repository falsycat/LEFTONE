in vec2  v_uv;
in float v_aa;
in float v_range;
in vec2  v_period;
in vec4  v_color;

out vec4 o_color;

vec2  uv_;
float len_;
float theta_;

float atan2(in vec2 p) {
  return p.x == 0.? PI/2.*sign(p.y): atan(p.y, p.x);
}
float thetadist(in vec2 p, in float t) {
  float a = tan(t);
  float b = 1.;
  if (cos(t) == 0.) {
    a = 1.;
    b = 0.;
  }
  return abs(b*p.y-a*p.x)/sqrt(a*a+b*b);
}

float grid(in float n, in float ri, in float ro) {
  float u = PI/n;
  float t = floor(theta_/u)*u;

  float d = min(thetadist(uv_, t), thetadist(uv_, t+u));
  return
      step(ri, len_)*step(len_, ro)*
      smoothstep(v_aa, .0, d);
}

float belt(in float t1, in float t2, in float r) {
  float t1a = smoothstep(v_aa, 0., thetadist(uv_, t1))*step(abs(theta_-t1), .1);
  float t2a = smoothstep(v_aa, 0., thetadist(uv_, t2))*step(abs(theta_-t2), .1);

  float a = t1a + t2a + step(t1, theta_)*step(theta_, t2);
  a *=
      (t1a + t2a)*smoothstep(.4, .45, len_) +
      smoothstep(r-v_aa, r, len_)*
      (1.-max(len_-r, 0.)/.2) +
      smoothstep(v_aa, 0., abs(len_-.7*r));
  return clamp(a, 0., 1.);
}

float circle(in float r) {
  return
      smoothstep(v_aa, 0., abs(len_-r));
}

float clockhand(in float t) {
  float r = smoothstep(.25, .3, len_)*smoothstep(1., .85, len_);
  float a =
      r *
      smoothstep(v_aa, 0., thetadist(uv_, t)) *
      step(abs(theta_-t), .1);
  float b =
      r * max(1.-abs(theta_-t)/(PI/12.), 0.)*.7;

  return a + b;
}

void main(void) {
  uv_    = v_uv.yx;
  len_   = length(uv_);
  theta_ = atan2(uv_);
  theta_ = theta_ < 0.? theta_+2.*PI: theta_;

  if (v_range <= -1.) {
    o_color = v_color;
    float a =
        grid( 2.,  .4, 1.) +
        grid(60., .75, .8) +
        grid(10., .65, .8) +
        grid(8.,  .5, .55) +
        grid( 4.,  .3, .4) +
        circle(.8);
    o_color.a *= clamp(a, 0., 1.);

  } else if (v_range <= 0.) {
    o_color   = v_color*clockhand(v_period.x*PI*2.);
    o_color.a = pow(o_color.a, 2.);

  } else {
    o_color    = v_color;
    o_color.a *= belt(v_period.x*PI*2., v_period.y*PI*2., v_range);
  }
}
