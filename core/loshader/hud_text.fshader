layout (location = 0) uniform sampler2D u_tex;
layout (location = 1) uniform float     u_alpha;

in vec2  v_pos;
in vec2  v_size;
in vec2  v_uv_pos;
in vec2  v_uv_size;
in vec2  v_dp;
in vec4  v_color;

out vec4 o_color;

void main(void) {
  vec2 dp = v_dp;
  dp     *= pow(length(dp), .1);

  vec2 uvp = (dp - v_pos) / v_size;
  vec2 uv  = uvp*v_uv_size + v_uv_pos;
  uvp = abs(uvp);

  float a =
      texture(u_tex, uv).r *
      step(abs(uvp.x-.5), .5) *
      step(abs(uvp.y-.5), .5);

  o_color    = v_color;
  o_color.a *= a*u_alpha;
}
