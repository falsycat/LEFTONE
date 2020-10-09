layout (location = 0) uniform sampler2D u_tex;
layout (location = 1) uniform float     u_alpha;

in vec2 v_dp;
in vec2 v_uv;
in vec4 v_color;

out vec4 o_color;

float rand(in vec2 p) {
  /* https://qiita.com/shimacpyon/items/d15dee44a0b8b3883f76 */
  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}
float noise(vec2 x) {
  /* https://www.shadertoy.com/view/4dS3Wd */
  vec2 i = floor(x);
  vec2 f = fract(x);

  float a = rand(i);
  float b = rand(i + vec2(1.0, 0.0));
  float c = rand(i + vec2(0.0, 1.0));
  float d = rand(i + vec2(1.0, 1.0));

  vec2 u = f * f * (3.0 - 2.0 * f);
  return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main(void) {
  float t = 1.-pow(1.-u_alpha, 4.);

  float a = texture(u_tex, v_uv).r;
  a *= step(noise(v_dp*10.), t);

  o_color = vec4(v_color.rgb, v_color.a*a*t);
}
