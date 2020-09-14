layout (location = 0) in vec2  i_pos;
layout (location = 1) in vec2  i_size;
layout (location = 2) in vec4  i_bgcolor;
layout (location = 3) in vec4  i_fgcolor;
layout (location = 4) in float i_value;
layout (location = 5) in float i_prev_value;

out vec2  v_pos;
out vec2  v_size;
out vec2  v_aa;
out vec2  v_dp;
out vec4  v_color;

void main() {
  const vec2[] verts = vec2[](
      vec2(-1.,  1.),
      vec2(-1., -1.),
      vec2( 1., -1.),
      vec2(-1.,  1.),
      vec2( 1., -1.),
      vec2( 1.,  1.)
  );

  float scale = (uni.cam * vec4(1., 0., 0., 0.)).x;
  scale = pow(1.2, scale-1.);

  v_pos  = i_pos;
  v_size = i_size;

  int id = 0;
  if (gl_VertexID < 6) {
    id      = gl_VertexID;
    v_size += vec2(2.)/uni.resolution;
    v_color = i_bgcolor;
  } else if (gl_VertexID < 12) {
    id        = gl_VertexID - 6;
    v_pos.x  -= v_size.x*(1.-i_prev_value);
    v_size.x *= i_prev_value;
    v_color   = mix(i_bgcolor, i_fgcolor, .5);
  } else {
    id        = gl_VertexID - 12;
    v_pos.x  -= v_size.x*(1.-i_value);
    v_size.x *= i_value;
    v_color   = i_fgcolor;
  }
  v_pos  *= scale;
  v_size *= scale;

  v_size = abs(v_size);
  v_aa   = uni.aa / v_size;

  v_dp  = verts[id]*1.5*v_size + v_pos;
  v_dp /= pow(length(v_dp), .1);

  gl_Position = vec4(v_dp, 0., 1.);
}
