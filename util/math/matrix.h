#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "./vector.h"

typedef union {
  vec2_t col[2];
  float  elm[2][2];
  float  ptr[4];
} mat2_t;

typedef union {
  vec3_t col[3];
  float  elm[3][3];
  float  ptr[9];
} mat3_t;

typedef union {
  vec4_t col[4];
  float  elm[4][4];
  float  ptr[16];
} mat4_t;

#define mat3_identity() (mat3_t) {{  \
    vec3(1, 0, 0),  \
    vec3(0, 1, 0),  \
    vec3(0, 0, 1),  \
}}
#define mat4_identity() (mat4_t) {{  \
    vec4(1, 0, 0, 0),  \
    vec4(0, 1, 0, 0),  \
    vec4(0, 0, 1, 0),  \
    vec4(0, 0, 0, 1),  \
}}

#define mat3_translation(x, y) (mat3_t) {{  \
    vec3(1, 0, 0),  \
    vec3(0, 1, 0),  \
    vec3(x, y, 1),  \
}}
#define mat4_translation(x, y, z) (mat4_t) {{  \
    vec4(1, 0, 0, 0),  \
    vec4(0, 1, 0, 0),  \
    vec4(0, 0, 1, 0),  \
    vec4(x, y, z, 1),  \
}}

#define mat3_scale(x, y) (mat3_t) {{  \
    vec3(x, 0, 0),  \
    vec3(0, y, 0),  \
    vec3(0, 0, 1),  \
}}
#define mat4_scale(x, y, z) (mat4_t) {{  \
    vec4(x, 0, 0, 0),  \
    vec4(0, y, 0, 0),  \
    vec4(0, 0, z, 0),  \
    vec4(0, 0, 0, 1),  \
}}

#define mat3_rotation_z(sin_theta, cos_theta) (mat3_t) {{  \
    vec3(   cos_theta, sin_theta, 0), \
    vec3(-(sin_theta), cos_theta, 0), \
    vec3(           0,         0, 1), \
}}

#define mat4_rotation_x(sin_theta, cos_theta) (mat4_t) {{  \
    vec4(1,            0,         0, 0), \
    vec4(0,    cos_theta, sin_theta, 0), \
    vec4(0, -(sin_theta), cos_theta, 0), \
    vec4(0,            0,         0, 1), \
}}
#define mat4_rotation_y(sin_theta, cos_theta) (mat4_t) {{  \
    vec4(cos_theta, 0, -(sin_theta), 0), \
    vec4(        0, 1,            0, 0), \
    vec4(sin_theta, 0,    cos_theta, 0), \
    vec4(        0, 0,            0, 1), \
}}
#define mat4_rotation_z(sin_theta, cos_theta) (mat4_t) {{  \
    vec4(   cos_theta, sin_theta, 0, 0), \
    vec4(-(sin_theta), cos_theta, 0, 0), \
    vec4(           0,         0, 1, 0), \
    vec4(           0,         0, 0, 1), \
}}

bool mat2_valid(const mat2_t* x);
bool mat3_valid(const mat3_t* x);
bool mat4_valid(const mat4_t* x);

void mat2_add(mat2_t* x, const mat2_t* l, const mat2_t* r);
void mat3_add(mat3_t* x, const mat3_t* l, const mat3_t* r);
void mat4_add(mat4_t* x, const mat4_t* l, const mat4_t* r);

void mat2_addeq(mat2_t* x, const mat2_t* r);
void mat3_addeq(mat3_t* x, const mat3_t* r);
void mat4_addeq(mat4_t* x, const mat4_t* r);

void mat2_sub(mat2_t* x, const mat2_t* l, const mat2_t* r);
void mat3_sub(mat3_t* x, const mat3_t* l, const mat3_t* r);
void mat4_sub(mat4_t* x, const mat4_t* l, const mat4_t* r);

void mat2_subeq(mat2_t* x, const mat2_t* r);
void mat3_subeq(mat3_t* x, const mat3_t* r);
void mat4_subeq(mat4_t* x, const mat4_t* r);

void mat2_mul(mat2_t* x, const mat2_t* l, const mat2_t* r);
void mat3_mul(mat3_t* x, const mat3_t* l, const mat3_t* r);
void mat4_mul(mat4_t* x, const mat4_t* l, const mat4_t* r);

void mat2_mul_vec2(vec2_t* x, const mat2_t* l, const vec2_t* r);
void mat3_mul_vec3(vec3_t* x, const mat3_t* l, const vec3_t* r);
void mat4_mul_vec4(vec4_t* x, const mat4_t* l, const vec4_t* r);

float mat2_det(const mat2_t* x);
float mat3_det(const mat3_t* x);
float mat4_det(const mat4_t* x);

void mat3_cofactor(mat2_t* x, const mat3_t* r, size_t row, size_t col);
void mat4_cofactor(mat3_t* x, const mat4_t* r, size_t row, size_t col);

bool mat3_inv(mat3_t* x, const mat3_t* r);
bool mat4_inv(mat4_t* x, const mat4_t* r);
