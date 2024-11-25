#include <stdio.h>
#include <stdbool.h>
#include <graphx.h>
#include <tice.h>
#include <math.h>

#define MAX_DEPTH 6

struct {
    float minX, minY, maxX, maxY;
} ViewWindow = {
    .minX = -2.0f,
    .minY = -2.0f,
    .maxX = 2.0f,
    .maxY = 2.0f
};

float f(float x, float y) {
    return x * x + y * y - 1; // Circle of radius 1
}

// Gradient of the function (partial derivatives)
void gradient(float (*function)(float, float), float x, float y, float *grad_x, float *grad_y) {
    float h = 1e-5f; // Small step size for numerical differentiation
    *grad_x = (function(x + h, y) - function(x - h, y)) / (2.0f * h);
    *grad_y = (function(x, y + h) - function(x, y - h)) / (2.0f * h);
}

// Lipschitz constant for the function in a region
float compute_lipschitz_constant(float (*function)(float, float), float x_min, float x_max, float y_min, float y_max) {
    float max_grad = 0.0f;
    const int SAMPLES = 1;  // Number of samples in each dimension

    for (int i = 0; i < SAMPLES; i++) {
        for (int j = 0; j < SAMPLES; j++) {
            float x = x_min + (x_max - x_min) * i / (SAMPLES - 1);
            float y = y_min + (y_max - y_min) * j / (SAMPLES - 1);

            float grad_x, grad_y;
            gradient(function, x, y, &grad_x, &grad_y);
            float grad_magnitude = sqrtf(grad_x * grad_x + grad_y * grad_y);

            if (grad_magnitude > max_grad) {
                max_grad = grad_magnitude;
            }
        }
    }

    return max_grad;
}

void renderRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float)) {
    if (depth >= MAX_DEPTH || size < 0.01f) {
        // Convert coordinates to screen pixels
        float windowToPixelRatioX = (float)GFX_LCD_WIDTH / (ViewWindow.maxX - ViewWindow.minX);
        float windowToPixelRatioY = (float)GFX_LCD_HEIGHT / (ViewWindow.maxY - ViewWindow.minY);

        int x1 = (int)((x - ViewWindow.minX) * windowToPixelRatioX);
        int y1 = (int)((y - ViewWindow.minY) * windowToPixelRatioY);
        int width = (int)(size * windowToPixelRatioX) + 1;
        int height = (int)(size * windowToPixelRatioY) + 1;

        // Choose color based on the function value at the center
        float centerX = x + size / 2.0f;
        float centerY = y + size / 2.0f;
        gfx_SetColor(function(centerX, centerY) <= 0.0f ? 100 : 200);
        gfx_FillRectangle(x1, y1, width, height);
        return;
    }

    float x1 = x;
    float y1 = y;
    float x2 = x + size;
    float y2 = y + size;

    // Sample corners of the region
    float f1 = function(x1, y1);
    float f2 = function(x2, y1);
    float f3 = function(x1, y2);
    float f4 = function(x2, y2);

    // Find minimum and maximum sampled values
    float sampled_min = fminf(fminf(f1, f2), fminf(f3, f4));
    float sampled_max = fmaxf(fmaxf(f1, f2), fmaxf(f3, f4));

    // Calculate Lipschitz constant for this region
    float L = compute_lipschitz_constant(function, x1, x2, y1, y2);

    // Calculate region diameter in function space
    float region_diameter = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    // Calculate possible range of f in this region
    float possible_min = sampled_min - L * region_diameter;
    float possible_max = sampled_max + L * region_diameter;

    // Decide whether to subdivide
    if (possible_min <= 0.0f && possible_max >= 0.0f) {
        // Subdivide and render children
        float halfSize = size / 2.0f;
        uint8_t nextDepth = depth + 1;

        renderRegion(x, y, halfSize, nextDepth, function);
        renderRegion(x + halfSize, y, halfSize, nextDepth, function);
        renderRegion(x, y + halfSize, halfSize, nextDepth, function);
        renderRegion(x + halfSize, y + halfSize, halfSize, nextDepth, function);
    } else {
        if(possible_min > 0) return;
        // Convert coordinates to screen pixels
        float windowToPixelRatioX = (float)GFX_LCD_WIDTH / (ViewWindow.maxX - ViewWindow.minX);
        float windowToPixelRatioY = (float)GFX_LCD_HEIGHT / (ViewWindow.maxY - ViewWindow.minY);

        int x1_pix = (int)((x - ViewWindow.minX) * windowToPixelRatioX);
        int y1_pix = (int)((y - ViewWindow.minY) * windowToPixelRatioY);
        int width = (int)(size * windowToPixelRatioX) + 1;
        int height = (int)(size * windowToPixelRatioY) + 1;

        // Choose color based on the possible_min and possible_max
        gfx_SetColor(possible_min > 0.0f ? 200 : 100);
        gfx_FillRectangle(x1_pix, y1_pix, width, height);
    }
}

void renderImplicitFunction() {
    gfx_Begin();

    renderRegion(ViewWindow.minX, ViewWindow.minY,
                 fmaxf(ViewWindow.maxX - ViewWindow.minX, ViewWindow.maxY - ViewWindow.minY),
                 0, f);

    while (!os_GetCSC());
    gfx_End();
}

int main(void) {
    renderImplicitFunction();
    return 0;
}