layout (location = 0) in float i_bullet_id;
layout (location = 1) in vec2  i_pos;
layout (location = 2) in vec2  i_size;
layout (location = 3) in float i_theta;
layout (location = 4) in float i_time;
layout (location = 5) in vec4  i_color;

out vec2  v_aa;
out float v_bullet_id;
out vec2  v_uv;
out vec2  v_size;
out float v_time;
out vec4  v_color;

mat2 rot(in float theta) {
  float c = cos(theta);
  float s = sin(theta);
  return mat2(c, -s, s, c);
}

vec2 square(void) {
  const vec2[] verts = vec2[](
      vec2(-1.,  1.), vec2(-1., -1.), vec2( 1., -1.),
      vec2(-1.,  1.), vec2( 1., -1.), vec2( 1.,  1.)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}

vec2 triangle(void) {
  const vec2[] verts = vec2[](
      vec2(-1.,  1.), vec2(-1., -1.), vec2( 1.,  0.)
  );
  return gl_VertexID < verts.length()? verts[gl_VertexID]: vec2(0.);
}

void main(void) {
  v_bullet_id = i_bullet_id;
  v_time      = i_time;
  v_color     = i_color;

  v_aa = uni.aa / (uni.proj * uni.cam * vec4(i_size, 0., 0.)).xy;

  v_uv =
      i_bullet_id == 0.? square():
      i_bullet_id == 1.? square():
      i_bullet_id == 2.? triangle():
      vec2(0.);
  v_size = i_size;

  gl_Position = uni.proj * uni.cam *
      vec4(rot(-i_theta)*(v_uv*i_size) + i_pos, 0., 1.);
}
