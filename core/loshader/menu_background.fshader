layout (location = 0) uniform float u_alpha;

in vec2 v_uv;

out vec4 o_color;

float atan2(in vec2 p){
    return p.x == 0. ? sign(p.y)*PI/2. : atan(p.y, p.x);
}

float map(in vec2 p, in float t) {
  if (p.x >= 2.) return 0.;

  vec2 v = vec2(.3, .4 + t*.3);
  for (float i = 0.; i < 10.; ++i) {
      p  = abs(p) - v;
      v /= dot(p, p);
  }
  return abs(p.x);
}

void main(void) {
  vec2 uv = v_uv * uni.resolution.xy / (4.*uni.dpi);

  vec2 polar = vec2(length(uv), atan2(uv));
  polar.y = abs(mod(polar.y, PI/3.) - PI/6.);

  vec2 p = vec2(cos(polar.y), sin(polar.y))*polar.x;

  o_color = vec4(0, 0, 0, .7*u_alpha);

  float t       = 1.-pow(1.-u_alpha, 8.);
  float fractal = step(100.-t*100.+3., map(p*1.8, t)) * polar.x;
  o_color = mix(o_color, vec4(.4, .2, .2, .1), fractal);
}
