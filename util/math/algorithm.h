#pragma once

#include <math.h>
#include <stdint.h>

#include "./constant.h"

#define MATH_ABS(a) ((a) > 0? (a): -(a))

#define MATH_MAX(a, b) ((a) > (b)? (a): (b))
#define MATH_MIN(a, b) ((a) < (b)? (a): (b))

#define MATH_DIFF(a, b) (MATH_MAX(a, b) - MATH_MIN(a, b))

#define MATH_CLAMP(x, min, max) ((x) < (min)? (min): (x) > (max)? (max): (x))

#define MATH_SIGN(a) (a < 0? -1: a > 0? 1: 0)

#define MATH_FLOAT_EQUAL(a, b) (MATH_ABS((a) - (b)) < MATH_EPSILON)
#define MATH_FLOAT_VALID(a) (!isnan(a) && !isinf(a))


#define decl_next_power2_(type)  \
    type##_t math_##type##_next_power2(type##_t x)

decl_next_power2_(int8);
decl_next_power2_(int16);
decl_next_power2_(int32);
decl_next_power2_(int64);
decl_next_power2_(uint8);
decl_next_power2_(uint16);
decl_next_power2_(uint32);
decl_next_power2_(uint64);

#undef decl_next_power2_

#define decl_gcd_(type)  \
    type##_t math_##type##_gcd(type##_t x, type##_t y)

decl_gcd_(int8);
decl_gcd_(int16);
decl_gcd_(int32);
decl_gcd_(int64);
decl_gcd_(uint8);
decl_gcd_(uint16);
decl_gcd_(uint32);
decl_gcd_(uint64);

#undef decl_gcd_

#define decl_lcm_(type)  \
    type##_t math_##type##_lcm(type##_t x, type##_t y)

decl_lcm_(int8);
decl_lcm_(int16);
decl_lcm_(int32);
decl_lcm_(int64);
decl_lcm_(uint8);
decl_lcm_(uint16);
decl_lcm_(uint32);
decl_lcm_(uint64);

#undef decl_lcm_
