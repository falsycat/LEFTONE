out vec2 v_uv;

void main(void) {
  const vec2[] verts = vec2[](
    vec2(-1.,  1.), vec2(-1., -1.), vec2( 1., -1.),
    vec2(-1.,  1.), vec2( 1., -1.), vec2( 1.,  1.)
  );

  v_uv = gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);

  gl_Position = vec4(v_uv, 0., 1.);
}
