layout (location = 0) uniform sampler2D u_src;
layout (location = 1) uniform float     u_intensity;

in vec2 v_uv;

out vec4 o_color;

const int N = 4;
const int S = 20;

float luminance(in vec4 col) {
  return col.r*.3+col.g*.6+col.b*.1;
}

void main(void) {
  vec2  uv   = (v_uv + 1.)/2.;
  vec2  uvi  = uv * uni.resolution.y;
  float unit = 1.0/uni.resolution.y;

	o_color = texture(u_src, uv);

  int   darkest_y   = 0;
  float darkest_lum = 1.;
  int   brightest_y   = 0;
  float brightest_lum = 0.;

  int stride = int(uni.resolution.y/float(S));
  for (int i = 0; i < int(uni.resolution.y); i+=stride) {
    vec4 col = texture(u_src, vec2(uv.x, unit*float(i)));

    float lum = luminance(col);
    if (brightest_lum < lum) {
      brightest_y   = i;
      brightest_lum = lum;
    }
    if (darkest_lum > lum) {
      darkest_y   = i;
      darkest_lum = lum;
    }
  }
  int st = min(darkest_y, brightest_y);
  int ed = max(darkest_y, brightest_y);
  st = ed-int(float(ed-st)*u_intensity);

  if (int(uvi.y) <= st || int(uvi.y) >= ed) {
    return;
  }

  float lum_unit = (brightest_lum-darkest_lum) / float(N);
  stride /= 5;

  int count = st;
  for (int n = 0; n < N; ++n) {
    float n2   = float(darkest_y < brightest_y? n: N-n-1);
    float low  = lum_unit * n2 + darkest_lum;
    float high = low + lum_unit;

    for (int i = st; i < ed; i+=stride) {
      vec4 col = texture(u_src, vec2(uv.x, unit*float(i)));

      float lum = luminance(col);
      if (low <= lum && lum <= high) {
        count += stride;
        if (count >= int(uvi.y)) {
          o_color = col;
          return;
        }
      }
    }
  }
}
