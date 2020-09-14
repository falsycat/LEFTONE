#pragma once

#include <stdbool.h>

typedef union {
  struct { float x, y; };
  float ptr[2];
} vec2_t;

typedef union {
  struct { float x, y, z; };
  float ptr[3];

  vec2_t xy;
} vec3_t;

typedef union {
  struct { float x, y, z, w; };
  float ptr[4];

  vec2_t xy;
  vec3_t xyz;
} vec4_t;

#define vec2(x, y)       (vec2_t) {{x, y}}
#define vec3(x, y, z)    (vec3_t) {{x, y, z}}
#define vec4(x, y, z, w) (vec4_t) {{x, y, z, w}}

bool vec2_valid(const vec2_t* x);
bool vec3_valid(const vec3_t* x);
bool vec4_valid(const vec4_t* x);

void vec2_add(vec2_t* x, const vec2_t* l, const vec2_t* r);
void vec3_add(vec3_t* x, const vec3_t* l, const vec3_t* r);
void vec4_add(vec4_t* x, const vec4_t* l, const vec4_t* r);

void vec2_addeq(vec2_t* x, const vec2_t* r);
void vec3_addeq(vec3_t* x, const vec3_t* r);
void vec4_addeq(vec4_t* x, const vec4_t* r);

void vec2_sub(vec2_t* x, const vec2_t* l, const vec2_t* r);
void vec3_sub(vec3_t* x, const vec3_t* l, const vec3_t* r);
void vec4_sub(vec4_t* x, const vec4_t* l, const vec4_t* r);

void vec2_subeq(vec2_t* x, const vec2_t* r);
void vec3_subeq(vec3_t* x, const vec3_t* r);
void vec4_subeq(vec4_t* x, const vec4_t* r);

void vec2_mul(vec2_t* x, const vec2_t* l, float r);
void vec3_mul(vec3_t* x, const vec3_t* l, float r);
void vec4_mul(vec4_t* x, const vec4_t* l, float r);

void vec2_muleq(vec2_t* x, float r);
void vec3_muleq(vec3_t* x, float r);
void vec4_muleq(vec4_t* x, float r);

void vec2_div(vec2_t* x, const vec2_t* l, float r);
void vec3_div(vec3_t* x, const vec3_t* l, float r);
void vec4_div(vec4_t* x, const vec4_t* l, float r);

void vec2_diveq(vec2_t* x, float r);
void vec3_diveq(vec3_t* x, float r);
void vec4_diveq(vec4_t* x, float r);

float vec2_pow_length(const vec2_t* x);
float vec3_pow_length(const vec3_t* x);
float vec4_pow_length(const vec4_t* x);

float vec2_length(const vec2_t* x);
float vec3_length(const vec3_t* x);
float vec4_length(const vec4_t* x);

float vec2_dot(const vec2_t* l, const vec2_t* r);
float vec3_dot(const vec3_t* l, const vec3_t* r);
float vec4_dot(const vec4_t* l, const vec4_t* r);

float vec2_cross(const vec2_t* l, const vec2_t* r);
/* TODO(catfoot): add vec3_cross function */
