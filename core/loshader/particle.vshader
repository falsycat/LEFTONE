layout (location = 0) in float i_particle_id;
layout (location = 1) in vec2  i_pos;
layout (location = 2) in vec2  i_size;
layout (location = 3) in vec4  i_color;
layout (location = 4) in float i_time;

out vec2  v_uv;
out vec4  v_color;
out float v_time;

void main(void) {
  v_uv    = vec2(0.);
  v_color = i_color;
  v_time  = i_time;

  gl_Position = vec4(0.);
}
