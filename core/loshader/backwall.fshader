layout(std140) uniform param {
  float type;
  float prev_type;
  float transition;
} p;

in vec2 v_uv;

out vec4 o_color;

const float EPSILON = 1e-4;
const float INF     = 1e+2;

/* ---- UTILITY FUNCTIONS ---- */
vec3 get_ray_direction(in vec2 p, in vec2 s, float fov) {
  return normalize(vec3(p*s/2., s.y/tan(radians(fov)/2.)));
}
mat2 rot(in float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return mat2(c, -s, s, c);
}
#define get_normal(sdf, d, p)  \
    normalize(d - vec3(  \
        sdf(p-vec3(EPSILON, 0., 0.)),  \
        sdf(p-vec3(0., EPSILON, 0.)),  \
        sdf(p-vec3(0., 0., EPSILON))))

/* ---- SIGNED DISTANCE FUNCTIONS ---- */
/* https://iquilezles.org/www/articles/distfunctions/distfunctions.htm */
float sd_box( vec3 p, vec3 b ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.)) + min(max(q.x,max(q.y,q.z)),0.);
}
float sd_round_box( vec3 p, vec3 b, float r ) {
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

/* ---- SCENE: infinite boxes ---- */
float infinite_boxes_sd_scene(in vec3 p) {
  p = abs(p);
  p = mod(p, 2.) - 1.;
  return sd_box(p, vec3(.5));
}
vec3 infinite_boxes_raymarch(in vec2 uv, in vec3 eye) {
  vec3 dir = get_ray_direction(uv, uni.resolution, 60.);

  float h = 2., d, i;
  for (i = 0.; i < 100.; ++i) {
    d = infinite_boxes_sd_scene(eye + dir*h);
    if (d < EPSILON || h > 100.) break;
    h += d;
  }
  float a = 45./(i+1.)/h;
  return mix(vec3(.3, .3, .4), vec3(.6, .8, .7), clamp(a, 0., 1.));
}
vec4 infinite_boxes(void) {
  vec2 e2 = vec2(0., 1.) / uni.resolution;

  vec3 eye = vec3(0., fract(uni.time/60.)*2., 0.);

  eye.xy += fract(uni.pos.xy/2.)*2. + uni.pos.zw - vec2(.5, .5);

  vec3 col = infinite_boxes_raymarch(v_uv, eye);
  vec3 scol =
      infinite_boxes_raymarch(v_uv + e2, eye) +
      infinite_boxes_raymarch(v_uv - e2, eye);
  scol /= 2.;
  return vec4(mix(col, scol, .5), 1.);
}

/* ---- SCENE: HOLLOW MOUNTAINS ---- */
float hollow_mountains_sd_scene(in vec3 p, in vec3 v) {
  float s = 8., r;
  p = abs(mod(p, 2.) - 1.)*2.;
  for (float i = 0.; i < 5.; ++i) {
    p  = 1. - abs(p-v);
    r  = 1.4/dot(p, p);
    s *= r;
    p *= r;
  }
  return length(p)/s;
}
vec4 hollow_mountains(in vec3 color) {
  vec3 dir = get_ray_direction(v_uv, uni.resolution, 60.);

  vec3 eye = vec3(0., 0., 0.);
  eye.xy = fract(uni.pos.xy/2.)*2. + uni.pos.zw;

  float t = abs(fract(uni.time/20.)*2.-1.);
  t = t*t*(3.-2.*t);
  vec3 v = vec3(.9, 1., .94+t*.1);

  float h = 1.3, d, i;
  for (i = 0.; i < 60.; ++i) {
    d = hollow_mountains_sd_scene(eye + dir*h, v);
    if (d < EPSILON || h > 100.) break;
    h += d;
  }

  float a = 200./(i+1.)/h;
  return vec4(mix(vec3(.2, .2, .2), color, a), 1.);
}

/* ---- SCENE: JAIL ---- */
float jail_sd_scene(in vec3 p) {
  p = mod(p, 2.) - 1.;

  float dist = INF;
  for (float i = 0.; i < 7.; ++i) {
    float t = PI/6.*i*2.;

    vec3 p2 = p;
    p2.x   += cos(t)*.1;
    p2.xz  *= rot(t);
    p2.y   += .4*(i-3.);

    dist = min(dist, sd_box(p2, vec3(.19)));
  }
  return dist;
}
vec3 jail_raymarch(in vec2 uv, in vec3 eye) {
  vec3 dir = get_ray_direction(uv, uni.resolution.xy, 60.);

  float h = 1.6, d, i;
  for (i = 0.; i < 20.; ++i) {
    d = jail_sd_scene(eye + dir*h);
    if (d < EPSILON || h > INF) break;
    h += d;
  }
  vec3 p    = eye + dir*h;
  vec3 norm = get_normal(jail_sd_scene, d, p);

  float diffuse = clamp(dot(vec3(0., 0., 1.), norm), 0., 1.);

  float a = 3. + diffuse*.7;

  a /= pow(h*.3, 2.) * (i*.2+1.);
  a  = clamp(a, 0., 1.);

  return mix(vec3(.2, .3, .3), vec3(.4, .4, .5), a) + vec3(.7, .4, .4)*a;
}
vec4 jail(void) {
  vec2 e1 = vec2(1., 0.)/uni.resolution.xy;
  vec2 e2 = vec2(0., 1.)/uni.resolution.xy;

  vec3 eye = vec3(0., 0., -4.);

  eye.xy += fract(uni.pos.xy/2.)*2. + uni.pos.zw + vec2(.5, .5);

  vec3 col = jail_raymarch(v_uv, eye);
  vec3 scol =
      jail_raymarch(v_uv + e1 + e2, eye) +
      jail_raymarch(v_uv + e1 - e2, eye) +
      jail_raymarch(v_uv - e1 + e2, eye) +
      jail_raymarch(v_uv - e1 - e2, eye);
  scol /= 4.;
  return vec4(mix(col, scol, .5), 1.);
}

/* ---- SCENE: fabric ---- */
float fabric_sd_rope(in vec3 p) {
  p.yz *= rot(PI/8. + p.x*PI);
  return sd_round_box(p, vec3(2.5, .2, .2), .03);
}
float fabric_sd_scene(in vec3 p) {
  p  = mod(p,  4.) - 2.;

  vec3 p2 = p/sqrt(2.);
  p2.xy *= rot(PI/4.);
  p2.y   = mod(p2.y, 2.) - 1.;
  float dist = fabric_sd_rope(p2);

  p2 = p/sqrt(2.);
  p2.xy *= rot(-PI/4.);
  p2.y   = mod(p2.y, 2.) - 1.;
  dist = min(dist, fabric_sd_rope(p2));

  return dist;
}
vec4 fabric(void) {
  vec3 dir = get_ray_direction(v_uv, uni.resolution.xy, 80.);
  vec3 eye = vec3(0., 0., -4.);

  eye.xy += uni.pos.zw*4.;

  float h = 4.5, d, i;
  for (i = 0.; i < 50.; ++i) {
      d = fabric_sd_scene(eye + dir*h);
      if (d < EPSILON || h > INF) break;
      h += d;
  }

  vec3 norm = get_normal(fabric_sd_scene, h, eye + dir*h);
  float diffuse = clamp(dot(vec3(0., 0., -1.), norm), .5, 1.);

  float a = diffuse*.9 - .3;
  a /= pow(h*.08, 2.) * (i*.3+.5);
  a  = clamp(a, 0., 1.);

  vec3 col = mix(vec3(.25, .3, .2), vec3(.4, .6, .7), a) + vec3(1., .0, .0)*a;
  return vec4(col, 1.);
}

vec4 scene(in float type) {
  return
      type == 1.? infinite_boxes():
      type == 2.? hollow_mountains(vec3(.1, .4, .1)):
      type == 3.? hollow_mountains(vec3(.4, .1, .1)):
      type == 4.? jail():
      type == 5.? fabric():
      vec4(1.);
}
void main(void) {
  vec4 prev = vec4(0.), next = vec4(0.);
  if (p.transition > 0.) next = clamp(scene(p.type), 0., 1.);
  if (p.transition < 1.) prev = clamp(scene(p.prev_type), 0., 1.);
  o_color = mix(prev, next, p.transition);
}
