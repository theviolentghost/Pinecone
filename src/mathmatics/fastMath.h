#ifndef FAST_FIXED_POINT_H
#define FAST_FIXED_POINT_H

#include <stdint.h>

typedef int32_t fast_int;

extern int DECIMAL_BITS;
extern fast_int epsilon; //1e-4
int setFastIntDecimalBits(int bits);

#define INT_TO_FIXED(x) ((fast_int)((x) << DECIMAL_BITS))
#define FIXED_TO_INT(x) ((int)((x) >> DECIMAL_BITS))
#define FLOAT_TO_FIXED(x) ((fast_int)((x) * (1 << DECIMAL_BITS)))
#define FIXED_TO_FLOAT(x) ((float)(x) / (1 << DECIMAL_BITS))

#define FIXED_PI FLOAT_TO_FIXED(3.14159)
#define FIXED_TWO_PI FLOAT_TO_FIXED(6.28318)
#define FIXED_HALF_PI FLOAT_TO_FIXED(1.57079)
#define FIXED_LN_10 FLOAT_TO_FIXED(2.302585093) // ln(10)

#define FIXED_NEG(x) ((fast_int)(-(x)))
#define FIXED_MUL(x, y) ((fast_int)(((int32_t)(x) * (int32_t)(y)) >> DECIMAL_BITS))
#define FIXED_DIV(x, y) ((fast_int)((((int32_t)(x) << DECIMAL_BITS) / (int32_t)(y))))
#define FIXED_ADD(x, y) ((fast_int)((x) + (y)))
#define FIXED_SUB(x, y) ((fast_int)((x) - (y)))

fast_int fast_add(fast_int a, fast_int b);
fast_int fast_sub(fast_int a, fast_int b);
fast_int fast_mul(fast_int a, fast_int b);
fast_int fast_div(fast_int a, fast_int b);
fast_int fast_sin(fast_int x);
fast_int fast_cos(fast_int x);
fast_int fast_tan(fast_int x);
fast_int fast_sec(fast_int x);
fast_int fast_csc(fast_int x);
fast_int fast_cot(fast_int x);
fast_int fast_root(fast_int x, int n);
fast_int fast_pow(fast_int x, int n);
fast_int fast_log_base(fast_int x, fast_int base);
fast_int fast_log(fast_int x);
fast_int fast_ln(fast_int x);
fast_int fast_exp(fast_int x);
fast_int fast_sqrt(fast_int x);
fast_int fast_abs(fast_int x);
fast_int fast_min(fast_int x, fast_int y);
fast_int fast_max(fast_int x, fast_int y);
fast_int fast_clamp(fast_int x, fast_int min, fast_int max);
fast_int fast_floor(fast_int x);
fast_int fast_ceil(fast_int x);
fast_int fast_round(fast_int x);
fast_int fast_sign(fast_int x);
fast_int fast_lerp(fast_int a, fast_int b, fast_int t);
fast_int fast_inverse_lerp(fast_int a, fast_int b, fast_int x);
fast_int fast_atan2(fast_int y, fast_int x);
fast_int fast_asin(fast_int x);
fast_int fast_acos(fast_int x);
fast_int fast_atan(fast_int x);
fast_int fast_asec(fast_int x);
fast_int fast_acsc(fast_int x);
fast_int fast_acot(fast_int x);

#endif