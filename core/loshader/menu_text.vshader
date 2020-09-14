layout (location = 0) in vec2 i_pos;
layout (location = 1) in vec2 i_size;
layout (location = 2) in vec2 i_uv_pos;
layout (location = 3) in vec2 i_uv_size;
layout (location = 4) in vec4 i_color;

out vec2 v_uv;
out vec4 v_color;

void main(void) {
  const vec2[] verts = vec2[](
    vec2( 0.,  0.), vec2( 0., -1.), vec2( 1., -1.),
    vec2( 0.,  0.), vec2( 1., -1.), vec2( 1.,  0.)

  );

  vec2 p  = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
  vec2 dp = p*i_size + i_pos;

  gl_Position = vec4(dp, 0, 1);
  v_uv        = p*i_uv_size + i_uv_pos;
  v_color     = i_color;
}

