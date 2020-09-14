out vec2 v_uv;

const vec2[6] square_ = vec2[](
    vec2(-1.,  1.),
    vec2(-1., -1.),
    vec2( 1., -1.),
    vec2(-1.,  1.),
    vec2( 1., -1.),
    vec2( 1.,  1.)
);

void main(void) {
  v_uv        = square_[gl_VertexID];
  gl_Position = vec4(v_uv, 0., 1.);
}
