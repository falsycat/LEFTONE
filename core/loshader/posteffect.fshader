layout (location = 0) uniform sampler2D u_src;

layout(std140) uniform param {
  float whole_blur;
  float raster;

  float radial_displacement;
  float amnesia_displacement;

  float radial_fade;

  float brightness;
} p;

in vec2 v_uv;

out vec4 o_color;

float atan2(in vec2 p){
  return p.x == 0. ? sign(p.y)*PI/2. : atan(p.y, p.x);
}
vec2 to_polar(in vec2 p) {
  return vec2(length(p), atan2(p));
}
vec2 to_rectangular(in vec2 p) {
  return vec2(cos(p.y)*p.x, sin(p.y)*p.x);
}
float rand(in vec2 p) {
  /* https://thebookofshaders.com/13/?lan=jp */
  return fract(sin(dot(p.xy, vec2(12.9898, 78.233)))*43758.5453123);
}
float noise (in vec2 _st) {
  /* https://thebookofshaders.com/13/?lan=jp*/
  vec2 i = floor(_st);
  vec2 f = fract(_st);

  float a = rand(i);
  float b = rand(i + vec2(1.0, 0.0));
  float c = rand(i + vec2(0.0, 1.0));
  float d = rand(i + vec2(1.0, 1.0));

  vec2 u = f * f * (3.0 - 2.0 * f);

  return
      mix(a, b, u.x) +
      (c - a)* u.y * (1.0 - u.x) +
      (d - b) * u.x * u.y;
}
float fbm (in vec2 _st) {
  /* https://thebookofshaders.com/13/?lan=jp*/
  const float octaves = 5;

  float v     = 0.0;
  float a     = 0.5;
  vec2  shift = vec2(100.0);
  mat2  rot   = mat2(cos(0.5), sin(0.5), -sin(0.5), cos(0.50));

  for (int i = 0; i < octaves; ++i) {
    v += a * noise(_st);
    _st = rot * _st * 2.0 + shift;
    a *= 0.5;
  }
  return v;
}

vec2 radial_displacement(in vec2 polar) {
  float intensity = p.radial_displacement + 1.;
  float r         = polar.x;

  const float sqrt2 = sqrt(2.);
  r = (1. - pow(abs(1. - r/sqrt2), intensity))*sqrt2 / intensity;

  return vec2(r, polar.y);
}
vec2 amnesia_displacement(in vec2 uv) {
  vec2 auv = abs(uv);
  float intensity = p.amnesia_displacement*(1.-pow(max(auv.x, auv.y), 2.));
  return uv + fbm(uv)*intensity - intensity/2.;
}

float radial_fade(in float len) {
  float intensity = p.radial_fade;
  return clamp(1. - intensity * max(len - (1.-intensity), 0.), 0., 1.);
}

vec4 chromatic_aberration(in vec2 uv) {
  float a = length(v_uv)/sqrt(2.);
  vec2  e = 1./uni.resolution*a;

  return vec4(
      texture(u_src, uv+e).r,
      texture(u_src, uv).g,
      texture(u_src, uv-e).b,
      1.);
}

vec4 blur(in vec2 uv) {
  vec2 e1 = vec2(1./uni.resolution.x, 0.);
  vec2 e2 = vec2(0., 1./uni.resolution.y);

  vec4 color =
    texture(u_src, uv+e1+.0) +
    texture(u_src, uv+e1+e2) +
    texture(u_src, uv+.0+e2) +
    texture(u_src, uv-e1+e2) +
    texture(u_src, uv-e1+.0) +
    texture(u_src, uv-e1-e2) +
    texture(u_src, uv+.0-e2) +
    texture(u_src, uv+e1-e2);
  return color/8.;
}

void main(void) {
  vec2 uv = v_uv;

  /* transformation */
  vec2 polar = to_polar(uv);
  polar      = radial_displacement(polar);

  uv = to_rectangular(polar);
  uv = amnesia_displacement(uv);

  uv.x += sin(v_uv.y*PI*2./uni.aa*5.)*.05*p.raster;

  /* pixel manipulation */
  uv = (uv+1.)/2.;
  vec4 color = chromatic_aberration(uv);

  if (p.whole_blur > 0.) color = mix(color, blur(uv), p.whole_blur);

  /* blending */
  float a = radial_fade(polar.x);
  o_color = mix(vec4(0., 0., 0., 1), color, a);

  /* color manip */
  o_color  = pow(o_color, vec4(1.4));
  o_color  = min(o_color, vec4(max(o_color.r, max(o_color.g, o_color.b)))*.95);
  o_color *= p.brightness;
}
