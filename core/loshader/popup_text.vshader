layout (location = 1) uniform float u_alpha;

layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_size;
layout (location = 2) in vec2 i_uv_pos;
layout (location = 3) in vec2 i_uv_size;
layout (location = 4) in vec4 i_color;

out vec2 v_dp;
out vec2 v_uv;
out vec4 v_color;

const float RANDOM_POS = .1;

float rand(in vec2 p) {
  /* https://qiita.com/shimacpyon/items/d15dee44a0b8b3883f76 */
  return fract(sin(dot(p ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void) {
  const vec2[] verts = vec2[](
    vec2( 0.,  0.), vec2( 0., -1.), vec2( 1., -1.),
    vec2( 0.,  0.), vec2( 1., -1.), vec2( 1.,  0.)
  );

  vec2 seed = i_pos + float(gl_InstanceID);

  vec2 rpos = 1.-vec2(rand(seed.xy), rand(seed.yx))*2.;
  rpos.x /= 8.;
  rpos.y  = -abs(rpos.y);

  float t = pow(1.-u_alpha, 4.);

  rpos *= t*RANDOM_POS;

  vec2 p  = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  vec2 dp = p*i_size + i_pos+rpos;

  gl_Position = vec4(dp, 0, 1);
  v_dp        = dp;
  v_uv        = p*i_uv_size + i_uv_pos;
  v_color     = i_color;
}
