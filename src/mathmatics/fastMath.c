#include "fastMath.h"

fast_int fast_sin(fast_int x) {
    // Reduce angle using modulo
    x %= FIXED_TWO_PI;
    if (x > FIXED_PI) x -= FIXED_TWO_PI;

    // Faster approximation with fewer terms
    fast_int x2 = FIXED_MUL(x, x);
    fast_int x3 = FIXED_MUL(x2, x);
    fast_int x5 = FIXED_MUL(x3, x2);
    fast_int x7 = FIXED_MUL(x5, x2);

    return x - FIXED_DIV(x3, INT_TO_FIXED(6)) + 
           FIXED_DIV(x5, INT_TO_FIXED(120)) - 
           FIXED_DIV(x7, INT_TO_FIXED(5040));
}
fast_int fast_cos(fast_int x) {
    // Use sin(x + π/2)
    return fast_sin(x + FIXED_HALF_PI);
}
fast_int fast_tan(fast_int x) {
    return FIXED_DIV(fast_sin(x), fast_cos(x));
}
fast_int fast_sec(fast_int x) {
    return FIXED_DIV(INT_TO_FIXED(1), fast_cos(x));
}
fast_int fast_csc(fast_int x) {
    return FIXED_DIV(INT_TO_FIXED(1), fast_sin(x));
}
fast_int fast_cot(fast_int x) {
    return FIXED_DIV(fast_cos(x), fast_sin(x));
}
fast_int fast_root(fast_int x, int n) {
    if (x <= 0 || n <= 0) return 0;
    
    fast_int guess = x >> 1;
    fast_int prev_guess;
    
    do {
        prev_guess = guess;
        
        // Newton-Raphson method for n-th root
        fast_int power_guess = fast_pow(guess, n - 1);
        guess = ((n - 1) * guess + FIXED_DIV(x, power_guess)) / n;
    } while (prev_guess != guess);
    
    return guess;
}
fast_int fast_pow(fast_int x, int n) {
    if (n == 0) return INT_TO_FIXED(1);
    if (n == 1) return x;
    
    fast_int result = x;
    for (int i = 1; i < n; i++) {
        result = FIXED_MUL(result, x);
    }
    
    return result;
}
fast_int fast_log_base(fast_int x, fast_int base) {
    if (x <= 0 || base <= 0) return 0;
    return FIXED_DIV(fast_ln(x), fast_ln(base));
}
fast_int fast_log(fast_int x) {
    return FIXED_DIV(fast_ln(x), FIXED_LN_10);
}
fast_int fast_ln(fast_int x) {
    if (x <= 0) return 0;
    
    fast_int result = INT_TO_FIXED(0);
    fast_int term = INT_TO_FIXED(1);
    fast_int y = FIXED_DIV(x - INT_TO_FIXED(1), x + INT_TO_FIXED(1));
    fast_int y2 = FIXED_MUL(y, y);
    
    for (int i = 1; i < 100; i += 2) {
        result += FIXED_MUL(term, y) / i;
        term *= y2;
    }
    
    return FIXED_MUL(result, INT_TO_FIXED(2));
}
fast_int fast_exp(fast_int x) {
    fast_int result = INT_TO_FIXED(1);
    fast_int term = INT_TO_FIXED(1);
    
    for (int i = 1; i < 100; i++) {
        term = FIXED_MUL(term, x) / i;
        result += term;
    }
    
    return result;
}
fast_int fast_sqrt(fast_int x) {
    if (x <= 0) return 0;
    
    fast_int guess = x >> 1;
    fast_int prev_guess;
    
    do {
        prev_guess = guess;
        guess = (guess + FIXED_DIV(x, guess)) >> 1;
    } while (prev_guess != guess);
    
    return guess;
}
fast_int fast_abs(fast_int x) {
    return (x < 0) ? -x : x;
}
fast_int fast_min(fast_int x, fast_int y) {
    return (x < y) ? x : y;
}
fast_int fast_max(fast_int x, fast_int y) {
    return (x > y) ? x : y;
}
fast_int fast_clamp(fast_int x, fast_int min, fast_int max) {
    return fast_min(fast_max(x, min), max);
}
fast_int fast_floor(fast_int x) {
    return x & ~((1 << DECIMAL_PRECISION) - 1);
}
fast_int fast_ceil(fast_int x) {
    return (x & ~((1 << DECIMAL_PRECISION) - 1)) + (1 << DECIMAL_PRECISION);
}
fast_int fast_round(fast_int x) {
    return (x + (1 << (DECIMAL_PRECISION - 1))) & ~((1 << DECIMAL_PRECISION) - 1);
}
fast_int fast_sign(fast_int x) {
    return (x > 0) - (x < 0);
}
fast_int fast_lerp(fast_int a, fast_int b, fast_int t) {
    return a + FIXED_MUL(b - a, t);
}
fast_int fast_inverse_lerp(fast_int a, fast_int b, fast_int x) {
    return FIXED_DIV(x - a, b - a);
}
fast_int fast_atan2(fast_int y, fast_int x) {
    if (x == 0) {
        if (y > 0) return FIXED_HALF_PI;
        if (y < 0) return -FIXED_HALF_PI;
        return 0;
    }
    
    fast_int angle;
    fast_int z = FIXED_DIV(y, x);
    
    if (fast_abs(x) > fast_abs(y)) {
        angle = fast_atan(z);
        if (x < 0) {
            if (y < 0) angle -= FIXED_PI;
            else angle += FIXED_PI;
        }
    } else {
        angle = -fast_atan(FIXED_DIV(x, y));
        if (y < 0) angle -= FIXED_HALF_PI;
        else angle += FIXED_HALF_PI;
    }
    
    return angle;
}
fast_int fast_asin(fast_int x) {
    return fast_atan2(x, fast_sqrt(INT_TO_FIXED(1) - FIXED_MUL(x, x)));
}
fast_int fast_acos(fast_int x) {
    return FIXED_HALF_PI - fast_asin(x);
}
fast_int fast_atan(fast_int x) {
    fast_int y = INT_TO_FIXED(1);
    fast_int angle = INT_TO_FIXED(0);
    
    if (fast_abs(x) > y) {
        x = FIXED_DIV(y, x);
        angle = FIXED_HALF_PI;
    }
    
    fast_int term = x;
    fast_int x2 = FIXED_MUL(x, x);
    
    for (int i = 1; i < 100; i += 2) {
        angle += term / i;
        term = -FIXED_MUL(term, x2);
    }
    
    return angle;
}
fast_int fast_asec(fast_int x) {
    return fast_acos(FIXED_DIV(INT_TO_FIXED(1), x));
}
fast_int fast_acsc(fast_int x) {
    return fast_asin(FIXED_DIV(INT_TO_FIXED(1), x));
}
fast_int fast_acot(fast_int x) {
    return FIXED_HALF_PI - fast_atan(x);
}