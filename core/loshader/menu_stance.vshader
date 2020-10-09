layout (location = 0) in float i_id;
layout (location = 1) in vec2  i_pos;
layout (location = 2) in vec2  i_size;
layout (location = 3) in float i_alpha;

out float v_id;
out vec2  v_uv;
out float v_alpha;

const vec2[] frame_ = vec2[](
    vec2( 0.0,  1.0), vec2(-1.0,  0.0), vec2( 0.0,  0.9),
    vec2( 0.0,  0.9), vec2(-1.0,  0.0), vec2(-0.9,  0.0),
    vec2(-1.0,  0.0), vec2( 0.0, -1.0), vec2(-0.9,  0.0),
    vec2(-0.9,  0.0), vec2( 0.0, -1.0), vec2( 0.0, -0.9),
    vec2( 0.9,  0.0), vec2( 0.0, -0.9), vec2( 0.0, -1.0),
    vec2( 1.0,  0.0), vec2( 0.9,  0.0), vec2( 0.0, -1.0),
    vec2( 0.0,  1.0), vec2( 0.9,  0.0), vec2( 1.0,  0.0),
    vec2( 0.0,  1.0), vec2( 0.0,  0.9), vec2( 0.9,  0.0)
);

vec2 empty(in int id) {
  const vec2[] verts = vec2[](
      vec2( 0.,  1.), vec2(-1.,  0.), vec2( 0., -1.),
      vec2( 0.,  1.), vec2( 1.,  0.), vec2( 0., -1.)
  );
  return id < verts.length()? verts[id]: vec2(0.);
}

vec2 missionary(in int id) {
  const vec2[] verts = vec2[](
      vec2(-0.12,  0.52), vec2(-0.52,  0.24), vec2( 0.04,  0.40),
      vec2(-0.52,  0.24), vec2( 0.00,  0.00), vec2( 0.00,  0.12),
      vec2(-0.60,  0.08), vec2(-0.68,  0.00), vec2(-0.04, -0.36),
      vec2(-0.72,  0.00), vec2(-0.08, -0.52), vec2( 0.00, -0.40),
      vec2(-0.76, -0.04), vec2(-0.60, -0.36), vec2(-0.12, -0.56),
      vec2(-0.60, -0.40), vec2(-0.04, -0.96), vec2(-0.12, -0.60)
  );
  return
      id < verts.length()  ?  verts[id]:
      id < verts.length()*2?  verts[id-verts.length()]*vec2(-1., 1.):
      vec2(0.);
}
vec2 revolutioner(in int id) {
  const vec2[] verts = vec2[](
      vec2(-0.52,  0.24), vec2(-0.60,  0.20), vec2( 0.00, -0.80),
      vec2( 0.00,  0.36), vec2(-0.48,  0.08), vec2(-0.24, -0.16),
      vec2( 0.08,  0.64), vec2(-0.16,  0.40), vec2( 0.44,  0.36),
      vec2( 0.04,  0.24), vec2(-0.16, -0.12), vec2( 0.56,  0.28)
  );
  return id < verts.length()? verts[id]: vec2(0.);
}
vec2 unfinisher(in int id) {
  const vec2[] verts = vec2[](
      vec2( 0.00,  1.00), vec2(-0.08,  0.20), vec2( 0.08,  0.20),
      vec2(-0.08,  0.20), vec2(-0.40,  0.00), vec2(-0.40, -0.40),
      vec2(-0.40, -0.40), vec2( 0.00, -0.60), vec2( 0.40, -0.40),
      vec2( 0.08,  0.20), vec2( 0.40,  0.00), vec2( 0.40, -0.40)
  );
  return id < verts.length()? verts[id]: vec2(0.);
}
vec2 philosopher(in int id) {
  const vec2[] verts = vec2[](
      vec2( 0.117, -0.367), vec2( 0.000, -0.750), vec2( 0.300, -0.533),
      vec2( 0.117,  0.050), vec2(-0.083, -0.117), vec2( 0.367, -0.517),
      vec2(-0.050,  0.283), vec2(-0.133, -0.117), vec2( 0.117,  0.083),
      vec2( 0.500,  0.500), vec2(-0.050,  0.333), vec2( 0.150,  0.133),
      vec2( 0.250,  0.917), vec2(-0.233,  0.317), vec2( 0.500,  0.533),
      vec2( 0.217,  0.917), vec2(-0.300,  0.817), vec2(-0.283,  0.333),
      vec2(-0.333,  0.783), vec2(-0.500,  0.567), vec2(-0.333,  0.333)
  );
  return id < verts.length()? verts[id]*vec2(1.1, .9)+vec2(0, -.1): vec2(0.);
}

vec2 get_vertex(in int id) {
  return
      i_id == 1.? missionary(id):
      i_id == 2.? revolutioner(id):
      i_id == 3.? unfinisher(id):
      i_id == 4.? philosopher(id):
      empty(id);
}

void main(void) {
  v_uv = gl_VertexID < frame_.length()?
      frame_[gl_VertexID]: get_vertex(gl_VertexID - frame_.length())*.8;

  v_id    = i_id;
  v_alpha = i_alpha;

  gl_Position = vec4(v_uv*i_size + i_pos, 0., 1.);
}
