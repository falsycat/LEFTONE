layout(std140) uniform param {
  vec4  color;
  float size;
} p;

out vec4 o_color;

void main(void) {
  o_color = p.color;
}
