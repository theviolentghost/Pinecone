// #include "./fastMath.h"
// #include <stdio.h>

// fast_int line(fast_int x, fast_int y) {
//     return x - y;
// }
// fast_int hyperbola(fast_int x, fast_int y) {
//     return fast_mul(x,x) - fast_mul(y,y) - INT_TO_FIXED(1);
// }

// void gradient(fast_int (*function)(fast_int, fast_int), fast_int x, fast_int y, fast_int *grad_x, fast_int *grad_y) {
//     fast_int divisor = INT_TO_FIXED(2);
//     fast_int h = FLOAT_TO_FIXED(0.01);
//     if(h == 0) h = 1;
//     *grad_x = fast_div(
//         fast_sub(
//             function(fast_add(x, h), y),
//             function(fast_sub(x, h), y)
//         ),
//     divisor);
    
//     *grad_y = fast_div(
//         fast_sub(
//             function(x, fast_add(y, h)),
//             function(x, fast_sub(y, h))
//         ),
//     divisor);
// }
// fast_int lipschitzConstant(fast_int (*function)(fast_int, fast_int), fast_int x_min, fast_int x_max, fast_int y_min, fast_int y_max) {
//     fast_int GRADIANT_SIMILARITY_VALUE = FLOAT_TO_FIXED(0.0001);
//     fast_int max_grad = INT_TO_FIXED(-1024);
//     fast_int min_grad = INT_TO_FIXED(1024);
//     fast_int avg_grad = 0;
//     const int SAMPLES = 2; // squared

//     for (int i = 0; i < SAMPLES; i++) {
//         for (int j = 0; j < SAMPLES; j++) {
//             fast_int x = x_min + fast_div(fast_sub(x_max, x_min), INT_TO_FIXED((SAMPLES - 1) * i));
//             fast_int y = y_min + fast_div(fast_sub(y_max, y_min), INT_TO_FIXED((SAMPLES - 1) * j));

//             fast_int grad_x, grad_y;
            
//             gradient(function, x, y, &grad_x, &grad_y);
//             //printf("%d,%d\n", FIXED_TO_INT(grad_x), FIXED_TO_INT(grad_y));
//             printf("(%f) (%f)\n", FIXED_TO_FLOAT(fast_mul(grad_x, grad_x)), FIXED_TO_FLOAT(grad_x));

            

//             fast_int grad_magnitude = fast_sqrt(fast_add(fast_mul(grad_x, grad_x), fast_mul(grad_y, grad_y)));

//             avg_grad = fast_add(avg_grad, grad_magnitude);

//             max_grad = (grad_magnitude > max_grad) ? grad_magnitude : max_grad;
//             min_grad = (grad_magnitude < min_grad) ? grad_magnitude : min_grad;
//         }
//     }

//     printf("(%f) (%f)", FIXED_TO_FLOAT(max_grad), FIXED_TO_FLOAT(avg_grad));
    

//     return max_grad;
// }

// int main() {
//     setFastIntDecimalBits(25);

//     lipschitzConstant(line, FLOAT_TO_FIXED(-5), FLOAT_TO_FIXED(5), FLOAT_TO_FIXED(-5), FLOAT_TO_FIXED(5)); 


//     fast_int a = FLOAT_TO_FIXED(0.009766);
//     fast_int b = FLOAT_TO_FIXED(0.009766);
//     float result = FIXED_TO_FLOAT(fast_mul(a, b)); // Assuming fast_add is a function in fastMath.h
//     printf("Result of fast_add(%d, %d) = %f\n", a, b, result);
//     return 0;
// }