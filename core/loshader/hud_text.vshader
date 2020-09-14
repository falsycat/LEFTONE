layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_size;
layout (location = 2) in vec2 i_uv_pos;
layout (location = 3) in vec2 i_uv_size;
layout (location = 4) in vec4 i_color;

out vec2  v_pos;
out vec2  v_size;
out vec2  v_uv_pos;
out vec2  v_uv_size;
out vec2  v_dp;
out vec4  v_color;

void main(void) {
  const vec2[] verts = vec2[](
      vec2( 0.,  0.), vec2( 0., -1.), vec2( 1., -1.),
      vec2( 0.,  0.), vec2( 1., -1.), vec2( 1.,  0.)
  );

  float scale = (uni.cam * vec4(1., 0., 0., 0.)).x;
  scale = pow(1.2, scale-1.);

  v_pos     = i_pos  * scale;
  v_size    = i_size * scale;
  v_uv_pos  = i_uv_pos;
  v_uv_size = i_uv_size;
  v_color   = i_color;

  v_dp  = verts[gl_VertexID]*v_size*1.5 + v_pos;
  v_dp /= pow(length(v_dp), .1);

  gl_Position = vec4(v_dp, 0., 1.);
}
