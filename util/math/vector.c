#include "./vector.h"

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stddef.h>

#include "./algorithm.h"

bool vec2_valid(const vec2_t* x) {
  return x != NULL &&
      MATH_FLOAT_VALID(x->ptr[0]) &&
      MATH_FLOAT_VALID(x->ptr[1]);
}
bool vec3_valid(const vec3_t* x) {
  return x != NULL &&
      MATH_FLOAT_VALID(x->ptr[0]) &&
      MATH_FLOAT_VALID(x->ptr[1]) &&
      MATH_FLOAT_VALID(x->ptr[2]);
}
bool vec4_valid(const vec4_t* x) {
  return x != NULL &&
      MATH_FLOAT_VALID(x->ptr[0]) &&
      MATH_FLOAT_VALID(x->ptr[1]) &&
      MATH_FLOAT_VALID(x->ptr[2]) &&
      MATH_FLOAT_VALID(x->ptr[3]);
}

void vec2_add(vec2_t* x, const vec2_t* l, const vec2_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x + r->x;
  x->y = l->y + r->y;
}
void vec3_add(vec3_t* x, const vec3_t* l, const vec3_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x + r->x;
  x->y = l->y + r->y;
  x->z = l->z + r->z;
}
void vec4_add(vec4_t* x, const vec4_t* l, const vec4_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x + r->x;
  x->y = l->y + r->y;
  x->z = l->z + r->z;
  x->w = l->w + r->w;
}

void vec2_addeq(vec2_t* x, const vec2_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x += r->x;
  x->y += r->y;
}
void vec3_addeq(vec3_t* x, const vec3_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x += r->x;
  x->y += r->y;
  x->z += r->z;
}
void vec4_addeq(vec4_t* x, const vec4_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x += r->x;
  x->y += r->y;
  x->z += r->z;
  x->w += r->w;
}

void vec2_sub(vec2_t* x, const vec2_t* l, const vec2_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x - r->x;
  x->y = l->y - r->y;
}
void vec3_sub(vec3_t* x, const vec3_t* l, const vec3_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x - r->x;
  x->y = l->y - r->y;
  x->z = l->z - r->z;
}
void vec4_sub(vec4_t* x, const vec4_t* l, const vec4_t* r) {
  assert(x != NULL);
  assert(l != NULL);
  assert(r != NULL);

  x->x = l->x - r->x;
  x->y = l->y - r->y;
  x->z = l->z - r->z;
  x->w = l->w - r->w;
}

void vec2_subeq(vec2_t* x, const vec2_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x -= r->x;
  x->y -= r->y;
}
void vec3_subeq(vec3_t* x, const vec3_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x -= r->x;
  x->y -= r->y;
  x->z -= r->z;
}
void vec4_subeq(vec4_t* x, const vec4_t* r) {
  assert(x != NULL);
  assert(r != NULL);

  x->x -= r->x;
  x->y -= r->y;
  x->z -= r->z;
  x->w -= r->w;
}

void vec2_mul(vec2_t* x, const vec2_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x * r;
  x->y = l->y * r;
}
void vec3_mul(vec3_t* x, const vec3_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x * r;
  x->y = l->y * r;
  x->z = l->z * r;
}
void vec4_mul(vec4_t* x, const vec4_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x * r;
  x->y = l->y * r;
  x->z = l->z * r;
  x->w = l->w * r;
}

void vec2_muleq(vec2_t* x, float r) {
  assert(x != NULL);

  x->x *= r;
  x->y *= r;
}
void vec3_muleq(vec3_t* x, float r) {
  assert(x != NULL);

  x->x *= r;
  x->y *= r;
  x->z *= r;
}
void vec4_muleq(vec4_t* x, float r) {
  assert(x != NULL);

  x->x *= r;
  x->y *= r;
  x->z *= r;
  x->w *= r;
}

void vec2_div(vec2_t* x, const vec2_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x / r;
  x->y = l->y / r;
}
void vec3_div(vec3_t* x, const vec3_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x / r;
  x->y = l->y / r;
  x->z = l->z / r;
}
void vec4_div(vec4_t* x, const vec4_t* l, float r) {
  assert(x != NULL);
  assert(l != NULL);

  x->x = l->x / r;
  x->y = l->y / r;
  x->z = l->z / r;
  x->w = l->w / r;
}

void vec2_diveq(vec2_t* x, float r) {
  assert(x != NULL);

  x->x /= r;
  x->y /= r;
}
void vec3_diveq(vec3_t* x, float r) {
  assert(x != NULL);

  x->x /= r;
  x->y /= r;
  x->z /= r;
}
void vec4_diveq(vec4_t* x, float r) {
  assert(x != NULL);

  x->x /= r;
  x->y /= r;
  x->z /= r;
  x->w /= r;
}

float vec2_pow_length(const vec2_t* x) {
  assert(x != NULL);
  return x->x*x->x + x->y*x->y;
}
float vec3_pow_length(const vec3_t* x) {
  assert(x != NULL);
  return x->x*x->x + x->y*x->y + x->z*x->z;
}
float vec4_pow_length(const vec4_t* x) {
  assert(x != NULL);
  return x->x*x->x + x->y*x->y + x->z*x->z + x->w*x->w;
}

float vec2_length(const vec2_t* x) {
  assert(x != NULL);
  return sqrtf(vec2_pow_length(x));
}
float vec3_length(const vec3_t* x) {
  assert(x != NULL);
  return sqrtf(vec3_pow_length(x));
}
float vec4_length(const vec4_t* x) {
  assert(x != NULL);
  return sqrtf(vec4_pow_length(x));
}

float vec2_dot(const vec2_t* l, const vec2_t* r) {
  assert(l != NULL);
  assert(r != NULL);
  return l->x*r->x + l->y*r->y;
}
float vec3_dot(const vec3_t* l, const vec3_t* r) {
  assert(l != NULL);
  assert(r != NULL);
  return l->x*r->x + l->y*r->y + l->z*r->z;
}
float vec4_dot(const vec4_t* l, const vec4_t* r) {
  assert(l != NULL);
  assert(r != NULL);
  return l->x*r->x + l->y*r->y + l->z*r->z + l->w*r->w;
}

float vec2_cross(const vec2_t* l, const vec2_t* r) {
  assert(l != NULL);
  assert(r != NULL);
  return l->x*r->y - l->y*r->x;
}
