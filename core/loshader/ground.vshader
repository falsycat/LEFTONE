layout (location = 0) in float i_ground_id;
layout (location = 1) in vec2  i_pos;
layout (location = 2) in vec2  i_size;

out float v_ground_id;
out vec2  v_size;
out vec2  v_uv;

const vec2[] square_ = vec2[](
    vec2(-1.,  1.), vec2(-1., -1.), vec2( 1., -1.),
    vec2(-1.,  1.), vec2( 1., -1.), vec2( 1.,  1.)
);

void main(void) {
  v_ground_id = i_ground_id;
  v_size      = i_size;
  v_uv        = square_[gl_VertexID];
  gl_Position = uni.proj * uni.cam * vec4(v_uv*i_size+i_pos, 0., 1.);
}

