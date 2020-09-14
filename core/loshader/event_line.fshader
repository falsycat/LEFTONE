layout (location = 0) uniform sampler2D u_tex;

in vec2 v_uv;
in vec4 v_color;

out vec4 o_color;

void main(void) {
  float a = texture(u_tex, v_uv).r;
  o_color = vec4(v_color.rgb, v_color.a*a);
}
