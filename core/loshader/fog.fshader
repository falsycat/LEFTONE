layout(std140) uniform param {
  float type;
  float prev_type;
  float transition;

  float bounds_fog;
  vec4  bounds_pos;  /* xy: chunk, zw: fract */
  vec2  bounds_size;
} p;

in vec2 v_pos;
in vec2 v_uv;

out vec4 o_color;

const float EPSILON = 1e-4;

/* ---- UTILITY FUNCTIONS ---- */
vec3 get_ray_direction(in vec2 p, in vec2 s, float fov) {
  return normalize(vec3(p*s/2., s.y/tan(radians(fov)/2.)));
}
float rand(in vec2 p) {
  /* https://qiita.com/shimacpyon/items/d15dee44a0b8b3883f76 */
  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}
float noise(in vec3 p) {
  /* https://www.shadertoy.com/view/4dS3Wd */
  const vec3 step = vec3(110, 241, 171);

  vec3 i = floor(p);
  vec3 f = fract(p);

  float n = dot(i, step);

  vec3 u = f * f * (3.0 - 2.0 * f);

  const vec3 e1 = vec3(1., 0., 0.);
  const vec3 e2 = vec3(0., 1., 0.);
  const vec3 e3 = vec3(0., 0., 1.);
  return mix(mix(mix(rand(dot(step, vec3(0.))+vec2(n)), rand(dot(step, e1      )+vec2(n)), u.x),
                 mix(rand(dot(step,       e2)+vec2(n)), rand(dot(step, e1+e2   )+vec2(n)), u.x), u.y),
             mix(mix(rand(dot(step,       e3)+vec2(n)), rand(dot(step, e1+e3   )+vec2(n)), u.x),
                 mix(rand(dot(step,    e2+e3)+vec2(n)), rand(dot(step, e1+e2+e3)+vec2(n)), u.x), u.y), u.z);
}
float fbm15(in vec3 p) {
  float v = 0., a = .5, f = 0.;

  for (int i = 0; i < 15; ++i) {
    v += a*noise(p);
    p *= 2.;
    a *= .5;
  }
  return v;
}

/* ---- SCENE: white cloud ---- */
vec4 white_cloud(void) {
  vec3 dir = get_ray_direction(v_uv, uni.resolution, 60.);
  vec3 eye = vec3(0.);

  eye.xy += uni.pos.xy + uni.pos.zw;

  float a = 0.;
  for (float i = 1.; i <= 5.; ++i) {
    a += fbm15(eye + dir*i/3.) / pow(i, 2.);
  }
  return vec4(.6, .6, .5, clamp(pow(a, 8.), 0., .8));
}

/* ---- SCENE: bounds fog ---- */
vec4 bounds_fog(void) {
  float aa = uni.aa * 100;

  vec2 pos;
  pos.x = (p.bounds_pos.x - uni.pos.x) + (p.bounds_pos.z - uni.pos.z);
  pos.y = (p.bounds_pos.y - uni.pos.y) + (p.bounds_pos.y - uni.pos.y);
  pos   = (uni.proj * uni.cam * vec4(pos, 0., 1.)).xy;

  vec2 size = (uni.proj * uni.cam * vec4(p.bounds_size, 0., 0.)).xy;

  vec2 area_pos = v_uv - pos;

  float r =
      smoothstep(size.x-aa, size.x+aa, abs(area_pos.x)) +
      smoothstep(size.y-aa, size.y+aa, abs(area_pos.y));

  float a = fbm15(vec3(area_pos, abs(fract(uni.time/60.)*2.-1.)));

  return vec4(1.) * a * r * p.bounds_fog;
}

vec4 scene(in float type) {
  return
      type == 1.? white_cloud():
      vec4(0.);
}
void main(void) {
  vec4 prev = vec4(0.), next = vec4(0.);
  if (p.transition > 0.) next = clamp(scene(p.type), 0., 1.);
  if (p.transition < 1.) prev = clamp(scene(p.prev_type), 0., 1.);
  o_color = mix(prev, next, p.transition);

  o_color += bounds_fog();
}
