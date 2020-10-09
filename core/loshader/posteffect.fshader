layout (location = 0) uniform sampler2D u_src;

layout(std140) uniform param {
  float distortion_amnesia;
  float distortion_radial;
  float distortion_urgent;
  float raster_whole;

  float aberration_radial;
  float blur_whole;
  float brightness_whole;
  float fade_radial;
} p;

in vec2 v_uv;

out vec4 o_color;

/* ---- utilities ---- */
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
float noise(in vec2 _st) {
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
float fbm(in vec2 _st) {
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

/* ---- distortion effects ---- */
void distortion_amnesia(inout vec2 uv) {
  vec2 a = abs(uv);

  float i = p.distortion_amnesia*(1.-pow(max(a.x, a.y), 2.));
  uv += (fbm(uv)-.5)*i;
}
void distortion_radial(inout vec2 polar) {
  float i = p.distortion_radial + 1.;

  const float sqrt2 = sqrt(2.);
  polar.x = (1. - pow(abs(1. - polar.x/sqrt2), i))*sqrt2 / i;
}
void distortion_urgent(inout vec2 polar) {
  float i = p.distortion_urgent;
  polar.x += pow(polar.x, 2.)*sin(polar.y*20.)*.2*i;
  polar.x /= i+1.;
}
void raster_whole(inout vec2 uv) {
  uv.x += sin(v_uv.y*PI*2./uni.aa*5.)*.05*p.raster_whole;
}

/* ---- color effects ---- */
void aberration_radial(inout vec4 color, in vec2 uv) {
  float a = length(v_uv)/sqrt(2.);
  vec2  e = 4./uni.resolution*pow(a, 4.)*p.aberration_radial;

  color = vec4(
      texture(u_src, uv+e).r,
      color.g,
      texture(u_src, uv-e).b,
      1.);
}
void blur_whole(inout vec4 color, in vec2 uv) {
  if (p.blur_whole <= 0.) return;

  vec2 e1 = vec2(1./uni.resolution.x, 0.);
  vec2 e2 = vec2(0., 1./uni.resolution.y);

  vec4 neighbors =
    texture(u_src, uv+e1+.0) +
    texture(u_src, uv+e1+e2) +
    texture(u_src, uv+.0+e2) +
    texture(u_src, uv-e1+e2) +
    texture(u_src, uv-e1+.0) +
    texture(u_src, uv-e1-e2) +
    texture(u_src, uv+.0-e2) +
    texture(u_src, uv+e1-e2);
  color = mix(color, neighbors/8., p.blur_whole);
}
void contrast_whole(inout vec4 color) {
  color = pow(color, vec4(1.4));
}
void monochromize_whole(inout vec4 color) {
  color = min(color, vec4(max(color.r, max(color.g, color.b)))*.95);
}
void brightness_whole(inout vec4 color) {
  color *= p.brightness_whole;
}
void fade_radial(inout vec4 color, in vec2 polar) {
  float i = p.fade_radial;
  color.rgb *= clamp(1.-i*max(polar.x-(1.-i), 0.), 0., 1.);
}

void main(void) {
  vec2 uv = v_uv;

  /* distortion effect */
  distortion_amnesia(uv);

  vec2 polar = to_polar(uv);
  distortion_radial(polar);
  distortion_urgent(polar);
  uv = to_rectangular(polar);

  raster_whole(uv);

  /* color effect */
  uv = (uv+1.)/2.;
  o_color = texture(u_src, uv);

  aberration_radial(o_color, uv);
  blur_whole(o_color, uv);

  contrast_whole(o_color);
  monochromize_whole(o_color);
  brightness_whole(o_color);
  fade_radial(o_color, polar);
}
