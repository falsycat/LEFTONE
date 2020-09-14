#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static void next_power2(const char* type, size_t N) {
  assert(N > 0);

  /* https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2 */

  printf("%s%zu_t math_%s%zu_next_power2(%s%zu_t x) {", type, N, type, N, type, N);

  printf("--x;");
  for (size_t i = 1; i < N; i*=2) {
    printf("x |= x >> %zu;", i);
  }
  printf("return ++x;");
  printf("}\n");
}

static void gcd(const char* type, size_t N) {
  assert(N > 0);

  printf("%s%zu_t math_%s%zu_gcd(%s%zu_t x, %s%zu_t y) {", type, N, type, N, type, N, type, N);

  printf("assert(x > 0);");
  printf("assert(y > 0);");

  printf("if (x < y) return math_%s%zu_gcd(y, x);", type, N);

  printf("%s%zu_t z;", type, N);

  printf("while (y) {");
  printf("z = x%%y;");
  printf("x = y;");
  printf("y = z;");
  printf("}");

  printf("return x;}\n");
}

static void lcm(const char* type, size_t N) {
  assert(N > 0);

  printf("%s%zu_t math_%s%zu_lcm(%s%zu_t x, %s%zu_t y) {", type, N, type, N, type, N, type, N);

  printf("assert(x > 0);");
  printf("assert(y > 0);");

  printf("return x / math_%s%zu_gcd(x, y) * y;}\n", type, N);
}

int main(int argc, char** argv) {
  (void) argc, (void) argv;

  printf("#include \"./algorithm.h\"\n");

  printf("#include <assert.h>\n");
  printf("#include <stdint.h>\n");

  next_power2("int", 8);
  next_power2("int", 16);
  next_power2("int", 32);
  next_power2("int", 64);
  next_power2("uint", 8);
  next_power2("uint", 16);
  next_power2("uint", 32);
  next_power2("uint", 64);

  gcd("int", 8);
  gcd("int", 16);
  gcd("int", 32);
  gcd("int", 64);
  gcd("uint", 8);
  gcd("uint", 16);
  gcd("uint", 32);
  gcd("uint", 64);

  lcm("int", 8);
  lcm("int", 16);
  lcm("int", 32);
  lcm("int", 64);
  lcm("uint", 8);
  lcm("uint", 16);
  lcm("uint", 32);
  lcm("uint", 64);

  return EXIT_SUCCESS;
}
