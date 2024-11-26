#include <stdio.h>
#include <stdbool.h>
#include <graphx.h>
#include <tice.h>
#include <math.h>

#define MAX_DEPTH 7

float windowToPixelRatioX = 1;
float windowToPixelRatioY = 1;
float minimumPixelRatio = 1;

#define planeToScreenX(x) ((int)((x - ViewWindow.minX) * windowToPixelRatioX))
#define planeToScreenY(y) ((int)((y - ViewWindow.minY) * windowToPixelRatioY))

// Function prototypes
void processRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float));
void processChildOfRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float));
void renderRegion(float x, float y, float size, float f1, float f2, float f3, float f4);

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

void processChildOfRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float)) {
    float halfSize = size / 2.0f;
    uint8_t nextDepth = depth + 1;

    processRegion(x, y, halfSize, nextDepth, function);
    processRegion(x + halfSize, y, halfSize, nextDepth, function);
    processRegion(x, y + halfSize, halfSize, nextDepth, function);
    processRegion(x + halfSize, y + halfSize, halfSize, nextDepth, function);
}

void processRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float)) {
    if (depth >= MAX_DEPTH || size * minimumPixelRatio <= 1) {
        // render region 
        // should have contour line unless depth is 0
        if(depth == 0) return;

        float x1 = x;
        float y1 = y;
        float x2 = x + size;
        float y2 = y + size;

        float f1 = function(x1, y1);
        float f2 = function(x2, y1);
        float f3 = function(x1, y2);
        float f4 = function(x2, y2);

        renderRegion(x, y, size, f1, f2, f3, f4);
        return;
    }

    float x1 = x;
    float y1 = y;
    float x2 = x + size;
    float y2 = y + size;

    float f1 = function(x1, y1);
    float f2 = function(x2, y1);
    float f3 = function(x1, y2);
    float f4 = function(x2, y2);

    // Find minimum and maximum sampled values
    float sampled_min = fminf(fminf(f1, f2), fminf(f3, f4));
    float sampled_max = fmaxf(fmaxf(f1, f2), fmaxf(f3, f4));

    if(sampled_min <= 0 && sampled_max >= 0) {
        // region has contour line
        // skip Lipschitz constant calculation (expensive)
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

    // Calculate Lipschitz constant for this region
    float L = compute_lipschitz_constant(function, x1, x2, y1, y2);

    // Calculate region diameter in function space
    float region_diameter = sqrtf((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));

    // Calculate possible range of f in this region
    float possible_min = sampled_min - L * region_diameter;
    float possible_max = sampled_max + L * region_diameter;

   if(possible_min <= 0 && possible_max >= 0) {
        // region has contour line
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

    // has no contour line
    return;
}

float interpolate(float f1, float f2, float p1, float p2) {
    if (fabsf(f2 - f1) < 0.0001f) {
        // If function values are very close, return midpoint
        return (p1 + p2) * 0.5f;
    }
    float t = -f1 / (f2 - f1); // Assuming we are looking for the zero crossing (isoLevel = 0)
    return p1 + t * (p2 - p1);
}

void renderRegion(float x, float y, float size, float f1, float f2, float f3, float f4) {
    float isoLevel = 0.0f;  // Threshold for contour line
    int squareIndex = 0;

    // Determine the case index based on corner values
    if (f1 < isoLevel) squareIndex |= 1;
    if (f2 < isoLevel) squareIndex |= 2;
    if (f3 < isoLevel) squareIndex |= 4;
    if (f4 < isoLevel) squareIndex |= 8;

    // Early exit if the square is entirely inside or outside the contour
    if (squareIndex == 0 || squareIndex == 15)
        return;

    // Corner positions
    float x0 = x;
    float y0 = y;
    float x1 = x + size;
    float y1 = y + size;

    //gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), (int)(size * windowToPixelRatioX) + 1, (int)(size * windowToPixelRatioY) + 1);


    switch (squareIndex) {
        case 1: {
            // Case 1: Bottom-left corner inside the contour
            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float bottomX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(bottomX), planeToScreenY(y0)); 
            break;
        }
        case 2: {
            // Case 2: Bottom-right corner inside the contour
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);
            float bottomX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 3: {
            // Case 3: Bottom-left and bottom-right corners inside the contour
            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 4: {
            // Case 4: Top-right corner inside the contour
            float topX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        /*case 5:
            // Case 5: Bottom-left and top-right corners inside the contour
            break;
        case 6:
            // Case 6: Bottom-right and top-right corners inside the contour
            break;
        case 7:
            // Case 7: Bottom-left, bottom-right, and top-right corners inside the contour
            break;
        case 8:
            // Case 8: Top-left corner inside the contour
            break;
        case 9:
            // Case 9: Bottom-left and top-left corners inside the contour
            break;
        case 10:
            // Case 10: Bottom-right and top-left corners inside the contour
            break;
        case 11:
            // Case 11: Bottom-left, bottom-right, and top-left corners inside the contour
            break;
        case 12:
            // Case 12: Top-right and top-left corners inside the contour
            break;
        case 13:
            // Case 13: Bottom-left, top-right, and top-left corners inside the contour
            break;
        case 14:
            // Case 14: Bottom-right, top-right, and top-left corners inside the contour
            break;*/

        default:
            // Invalid case (should not happen)
            gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), (int)(size * windowToPixelRatioX) + 1, (int)(size * windowToPixelRatioY) + 1);
            break;
    }
}

void renderImplicitFunction() {
    windowToPixelRatioX = (float)GFX_LCD_WIDTH / (ViewWindow.maxX - ViewWindow.minX);
    windowToPixelRatioY = (float)GFX_LCD_HEIGHT / (ViewWindow.maxY - ViewWindow.minY);
    minimumPixelRatio = fminf(windowToPixelRatioX, windowToPixelRatioY);

    gfx_Begin();

    processRegion(ViewWindow.minX, ViewWindow.minY,
                 fmaxf(ViewWindow.maxX - ViewWindow.minX, ViewWindow.maxY - ViewWindow.minY),
                 0, f);

    while (!os_GetCSC());
    gfx_End();
}

int main(void) {
    renderImplicitFunction();
    return 0;
}