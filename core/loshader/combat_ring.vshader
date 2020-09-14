layout (location = 0) in float i_range;
layout (location = 1) in vec2  i_period;
layout (location = 2) in vec4  i_color;

out vec2  v_uv;
out float v_aa;
out float v_range;
out vec2  v_period;
out vec4  v_color;

void main() {
  v_uv =
      (gl_VertexID == 0)? vec2(-1.,  1.):
      (gl_VertexID == 1)? vec2(-1., -1.):
      (gl_VertexID == 2)? vec2( 1.,  1.):
      (gl_VertexID == 3)? vec2(-1., -1.):
      (gl_VertexID == 4)? vec2( 1., -1.):
      (gl_VertexID == 5)? vec2( 1.,  1.):
      vec2(0, 0);

  vec2 scale = uni.dpi*5. / uni.resolution;
  scale /= max(scale.x, 1.);
  scale /= max(scale.y, 1.);


  gl_Position = vec4(v_uv*scale, 0., 1.);
  v_aa        = length(uni.aa / scale)*2.;
  v_range     = i_range;
  v_period    = i_period;
  v_color     = i_color;
}
