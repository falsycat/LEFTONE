#undef NDEBUG

#include <assert.h>
#include <math.h>
#include <float.h>
#include <stdbool.h>
#include <stdio.h>

#include "./algorithm.h"
#include "./matrix.h"
#include "./rational.h"
#include "./vector.h"

static bool vec4_is_equal(vec4_t v1, vec4_t v2) {
  return
    MATH_FLOAT_EQUAL(v1.x, v2.x) &&
    MATH_FLOAT_EQUAL(v1.y, v2.y) &&
    MATH_FLOAT_EQUAL(v1.z, v2.z) &&
    MATH_FLOAT_EQUAL(v1.w, v2.w);
}

static void test_matrix() {
  vec4_t v1, v2;
  mat4_t m1, m2, m3;

  v1 = vec4(1, 1, 1, 1);
  m1 = mat4_identity();
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, v1));

  v1 = vec4(1, 1, 1, 1);
  m1 = mat4_translation(1, 1, 1);
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, vec4(2, 2, 2, 1)));

  v1 = vec4(1, 1, 1, 1);
  m1 = mat4_scale(.5f, 2, 1);
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, vec4(.5f, 2, 1, 1)));

  v1 = vec4(0, 1, 0, 1);
  m1 = mat4_rotation_x(-1 /* = sin -PI/2 */, 0 /* = cos -PI/2 */);
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, vec4(0, 0, -1, 1)));

  v1 = vec4(1, 0, 0, 1);
  m1 = mat4_rotation_y(1 /* = sin PI/2 */, 0 /* = cos PI/2 */);
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, vec4(0, 0, -1, 1)));

  v1 = vec4(0, 0, -1, 1);
  m1 = mat4_rotation_z(1 /* = sin PI/2 */, 0 /* = cos PI/2 */);
  mat4_mul_vec4(&v2, &m1, &v1);
  assert(vec4_is_equal(v2, vec4(0, 0, -1, 1)));

  v1 = vec4(-1, 1, 0, 1);
  m1 = mat4_rotation_z(1 /* = sin PI/2 */, 0 /* = cos PI/2 */);
  m2 = mat4_scale(.5f, .5f, .5f);
  mat4_mul(&m3, &m2, &m1);
  m1 = mat4_translation(1, 1, 0);
  mat4_mul(&m2, &m1, &m3);
  mat4_mul_vec4(&v2, &m2, &v1);
  assert(vec4_is_equal(v2, vec4(.5f, .5f, 0, 1)));
}

static void test_algorithm(void) {
  assert(math_int32_next_power2(8)   == 8);
  assert(math_int32_next_power2(20)  == 32);
  assert(math_int32_next_power2(200) == 256);

  assert(math_int32_gcd(10, 5)  == 5);
  assert(math_int32_gcd(12, 6)  == 6);
  assert(math_int32_gcd(12, 9)  == 3);
  assert(math_int32_gcd(12, 11) == 1);

  assert(math_int32_lcm(10, 5)  == 10);
  assert(math_int32_lcm(12, 10) == 60);
  assert(math_int32_lcm(7, 8)   == 56);
}

static void test_rational(void) {
  rational_t x, y;

  x = rational(1, 2);
  y = rational(1, 4);
  rational_addeq(&x, &y);
  assert(x.num == 3 && x.den == 4);

  x = rational(1, 2);
  y = rational(1, 3);
  rational_muleq(&x, &y);
  assert(x.num == 1 && x.den == 6);

  x = rational(10, 20);
  rational_simplify(&x);
  assert(x.num == 1 && x.den == 2);

  x = rational(1, 2);
  rational_normalize(&x, 100);
  assert(x.num == 50 && x.den == 100);

  x = rational(1, 2);
  assert(rational_calculate(&x) == 1.0f/2.0f);
}

int main(void) {
  test_matrix();
  test_algorithm();
  test_rational();
  return 0;
}
