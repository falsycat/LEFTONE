layout(std140) uniform param {
  vec4  color;
  float size;
} p;

const vec2[6] rect_ = vec2[](
    vec2(-1., 1.),
    vec2(-1., 0.),
    vec2( 1., 0.),
    vec2(-1., 1.),
    vec2( 1., 0.),
    vec2( 1., 1.)
);

void main(void) {
  int id = gl_VertexID < 6? gl_VertexID: gl_VertexID-6;

  vec2 p = rect_[id]*vec2(1., p.size) + vec2(0., 1.-p.size);
  p.y *= (id == gl_VertexID? 1.: -1.);

  gl_Position = vec4(p, 0., 1.);
}
