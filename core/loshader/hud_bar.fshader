in vec2 v_pos;
in vec2 v_size;
in vec2 v_aa;
in vec2 v_dp;
in vec4 v_color;

out vec4 o_color;

void main() {
  vec2 dp  = v_dp;
  dp      *= pow(length(dp), .1);

  vec2 diff = abs(dp - v_pos);
  float aa = uni.aa * 4.;
  float a =
      (1.-smoothstep(v_size.x-aa, v_size.x, diff.x)) *
      (1.-smoothstep(v_size.y-aa, v_size.y, diff.y));

  o_color    = v_color;
  o_color.a *= a;
}
