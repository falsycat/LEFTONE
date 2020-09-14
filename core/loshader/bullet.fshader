in vec2  v_aa;
in float v_bullet_id;
in vec2  v_uv;
in vec2  v_size;
in float v_time;
in vec4  v_color;

out vec4 o_color;

float dot(in vec2 p, in vec2 q) {
  return p.x*q.y - p.y*q.x;
}

vec4 light(void) {
  vec4 color = v_color;
  color.a *= 1.-pow(length(v_uv), 1.5);
  return color;
}

vec4 square(void) {
  vec4 color = v_color;

  float t = 1.-v_time;

  vec2 uv = abs(v_uv);
  color.a *= max(step(t-v_aa.x, uv.x), step(t-v_aa.y, uv.y));
  return color;
}

vec4 triangle(void) {
  const vec2 disp = vec2(-1./12., 0.);

  float t = 1. - v_time;
  vec2 v1 = vec2( 1.,  0.)*t + disp*v_time;
  vec2 v2 = vec2(-1., -1.)*t + disp*v_time;
  vec2 v3 = vec2(-1.,  1.)*t + disp*v_time;

  float b1 = dot(v_uv-v2, v1-v2);
  float b2 = dot(v_uv-v3, v2-v3);
  float b3 = dot(v_uv-v1, v3-v1);

  vec4 color = v_color;
  color.a *= b1*b2 > 0. && b2*b3 > 0.? 0.: 1.;
  return color;
}

void main(void) {
  o_color =
      v_bullet_id == 0.? light():
      v_bullet_id == 1.? square():
      v_bullet_id == 2.? triangle():
      vec4(0.);
}
