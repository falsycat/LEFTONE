in float v_id;
in vec2  v_uv;
in float v_alpha;

out vec4 o_color;

void main(void) {
  o_color = vec4(1., 1., 1., v_alpha*.8);
}
