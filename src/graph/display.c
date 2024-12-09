#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>
#include <ti/getcsc.h>
#include "../mathmatics/fastMath.h"

#define MAX_DEPTH 6

/*
float windowToPixelRatioX = 1;
float windowToPixelRatioY = 1;
float minimumPixelRatio = 1;

bool limitRefinement = false;

#define planeToScreenX(x) ((int)((x - ViewWindow.minX) * windowToPixelRatioX))
#define planeToScreenY(y) (GFX_LCD_HEIGHT - ((int)((y - ViewWindow.minY) * windowToPixelRatioY))) //flipped to account for math and computer science coordinate differences

// Function prototypes
void processRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float));
void processChildOfRegion(float x, float y, float size, uint8_t depth, float (*function)(float, float));
void renderRegion(float (*function)(float, float), float x, float y, float size, float f1, float f2, float f3, float f4);

struct {
    float minX, minY, maxX, maxY;
} ViewWindow = {
    .minX = -4.50f,
    .minY = -4.50f,
    .maxX = 6.0f,
    .maxY = 6.0f
};

float circle(float x, float y) {
    return x * x + y * y - 5; // Circle of radius 1
}
float hyperbola(float x, float y) {
    return x * x - y * y - 1; // Hyperbola
}
float line(float x, float y) {
    return x - y; 
}
float f2(float x, float y) {
    return x - 1/sinf(y) * .5; 
}

void gradient(float (*function)(float, float), float x, float y, float *grad_x, float *grad_y) {
    float h = 1e-4f; // Small step size for numerical differentiation
    // *grad_x = (function(x + h, y) - function(x - h, y)) / (2.0f * h);
    // *grad_y = (function(x, y + h) - function(x, y - h)) / (2.0f * h);
    *grad_x = (function(x + h, y) - function(x, y)) / h;
    *grad_y = (function(x, y + h) - function(x, y)) / h;
}

// Lipschitz constant for the function in a region
float compute_lipschitz_constant(float (*function)(float, float), float x_min, float x_max, float y_min, float y_max) {
    float max_grad = 0.0f;
    const int SAMPLES = 2;  // Number of samples in each dimension

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
    if (depth >= MAX_DEPTH || (size * minimumPixelRatio <= 1 && limitRefinement)) {
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

        renderRegion(function, x, y, size, f1, f2, f3, f4);
        return;
    }
    
    

    //if(size > 8) processChildOfRegion(x, y, size, depth, function); // if pixel region larger than 8 just process sub children

    float x1 = x;
    float y1 = y;
    float x2 = x + size;
    float y2 = y + size;

    float f1 = function(x1, y1);
    float f2 = function(x2, y1);
    float f3 = function(x1, y2);
    float f4 = function(x2, y2);

    
    if(depth < 3) {
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

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
    float region_diameter = size * 1.41421356237f * 2;  // sqrt(2)

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

void renderRegion(float (*function)(float, float), float x, float y, float size, float f1, float f2, float f3, float f4) {
    float isoLevel = 0.0f;  // Threshold for contour line
    int squareIndex = 0;

    // Determine the case index based on corner values
    if (f1 < isoLevel) squareIndex |= 1;
    if (f2 < isoLevel) squareIndex |= 2;
    if (f3 < isoLevel) squareIndex |= 4;
    if (f4 < isoLevel) squareIndex |= 8;

    // Early exit if the square is entirely inside or outside the contour
    if (squareIndex == 0 || squareIndex == 15) return;

    //squareIndex = 16;

    // Corner positions
    float x0 = x;
    float y0 = y;
    float x1 = x + size;
    float y1 = y + size;


    //gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), (int)(size * windowToPixelRatioX) + 1, (int)(size * windowToPixelRatioY) + 1);


    switch (squareIndex) {
        case 1: {
            // Case 1: Only f1 (top-left) is below the isoLevel
            // Line between edge 0 and edge 3

            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(topX), planeToScreenY(y0)); 
            break;
        }
        case 2: {
            // Case 2: Only f2 (top-right) is below the isoLevel
            // Line between edge 0 and edge 1

            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 3: {
            // Case 3: f1 and f2 are below the isoLevel
            // Line between edge 1 and edge 3

            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 4: {
            // Case 4: Only f3 (bottom-right) is below the isoLevel
            // Line between edge 1 and edge 2

            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);
            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x0), planeToScreenY(leftY));
            break;
        }
        case 5: {
            // Case 5: f1 and f3 are below the isoLevel
            // Lines between edge 0 to edge 3, and edge 1 to edge 2 (ambiguous case)

            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);
            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        case 6: {
            // Case 6: f2 and f3 are below the isoLevel
            // Line between edge 0 and edge 2

            
            break;
        }
        case 7: {
            // Case 7: f1, f2, and f3 are below the isoLevel
            // Line between edge 2 and edge 3

            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 8: {
            // Case 8: Only f4 (bottom-left) is below the isoLevel
            // Line between edge 2 and edge 3

            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 9: {
            // Case 9: f1 and f4 are below the isoLevel
            // Line between edge 0 and edge 2

            
            break;
        }
        case 10: {
            // Case 10: f2 and f4 are below the isoLevel
            // Lines between edge 0 to edge 1, and edge 2 to edge 3 (ambiguous case)

            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);
            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        case 11: { 
            // Case 11: f1, f2, and f4 are below the isoLevel
            // Line between edge 1 and edge 2

            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float bottomX = fminf(fmaxf(interpolate(f3, f4, x0, x1), x0), x1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        case 12: {
            // Case 12: f3 and f4 are below the isoLevel
            // Line between edge 1 and edge 3

            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 13: {
            // Case 13: f1, f3, and f4 are below the isoLevel
            // Line between edge 0 and edge 1

            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);
            float rightY = fminf(fmaxf(interpolate(f2, f4, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 14: {
            // Case 14: f2, f3, and f4 are below the isoLevel
            // Line between edge 0 and edge 3

            float topX = fminf(fmaxf(interpolate(f1, f2, x0, x1), x0), x1);
            float leftY = fminf(fmaxf(interpolate(f1, f3, y0, y1), y0), y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(topX), planeToScreenY(y0));
            break;
        }
        default:
            // Invalid case (should not happen)
            gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), (int)(size * windowToPixelRatioX) + 1, (int)(size * windowToPixelRatioY) + 1);
            break;
    }
}

void renderImplicitFunction(float (*function)(float, float), uint8_t color) {
    gfx_SetColor(color);
    processRegion(ViewWindow.minX, ViewWindow.minY, fmaxf(ViewWindow.maxX - ViewWindow.minX, ViewWindow.maxY - ViewWindow.minY), 0, function);
}

void startGraph() {
    

    windowToPixelRatioX = (float)GFX_LCD_WIDTH / (ViewWindow.maxX - ViewWindow.minX);
    windowToPixelRatioY = (float)GFX_LCD_HEIGHT / (ViewWindow.maxY - ViewWindow.minY);
    minimumPixelRatio = fminf(windowToPixelRatioX, windowToPixelRatioY);

    // draw grid
    // draw other things

    //then graphs themselves

    
}
*/














fast_int line(fast_int x, fast_int y) {
    return x - y;
}
fast_int hyperbola(fast_int x, fast_int y) {
    return fast_mul(x,x) - fast_mul(y,y) - INT_TO_FIXED(1);
}
fast_int csc(fast_int x, fast_int y) {
    return x - fast_div(INT_TO_FIXED(1), fast_sin(y));
}

fast_int GRADIANT_CUTOFF_VALUE; // if gradiant if below this value then stop refining

fast_int planeToScreenRatioX;
fast_int planeToScreenRatioY;
fast_int minimumPlaneToScreenRatio;

struct {
    fast_int minX, minY, maxX, maxY;
} ViewWindow;

int planeToScreenX(fast_int x) {
    return FIXED_TO_INT(fast_mul((x - ViewWindow.minX), planeToScreenRatioX));
}
int planeToScreenY(fast_int y) {
    return GFX_LCD_HEIGHT - FIXED_TO_INT(fast_mul((y - ViewWindow.minY), planeToScreenRatioY)); // flipped to account for math and computer science coordinate differences
}

void startGraph(void);
void processRegion(fast_int x, fast_int y, fast_int size, uint8_t depth, fast_int (*function)(fast_int, fast_int));
void processChildOfRegion(fast_int x, fast_int y, fast_int size, uint8_t depth, fast_int (*function)(fast_int, fast_int));
void renderRegion(fast_int (*function)(fast_int, fast_int), fast_int x, fast_int y, fast_int size, fast_int f1, fast_int f2, fast_int f3, fast_int f4);

uint8_t minimumDecimalBits = 8;
int calculateNeccessaryDecimalBits(fast_int minX,fast_int minY,fast_int maxX,fast_int maxY) {
    float minimumWindowRange = FIXED_TO_FLOAT(fast_min(maxX - minX, maxY - minY));
    return ((int)ceil(
        (31 - minimumDecimalBits) * expf(-minimumWindowRange / (32 / 2))
    ) + minimumDecimalBits);
}

void renderImplicitFunction(fast_int (*function)(fast_int, fast_int), uint8_t color) {
    gfx_SetColor(color);
    processRegion(ViewWindow.minX, ViewWindow.minY, fast_max(ViewWindow.maxX - ViewWindow.minX, ViewWindow.maxY - ViewWindow.minY), 0, function);
}

void startGraph(void) {
    fast_int minX = FLOAT_TO_FIXED(-10.0);
    fast_int minY = FLOAT_TO_FIXED(-10.0);
    fast_int maxX = FLOAT_TO_FIXED(10.0);
    fast_int maxY = FLOAT_TO_FIXED(10.0);    

    // calculateNeccessaryDecimalBits(minX, minY, maxX, maxY))
    setFastIntDecimalBits(14);

    GRADIANT_CUTOFF_VALUE = FLOAT_TO_FIXED(1);

    ViewWindow.minX = minX;
    ViewWindow.minY = minY;
    ViewWindow.maxX = maxX;
    ViewWindow.maxY = maxY;

    planeToScreenRatioX = fast_div(INT_TO_FIXED(GFX_LCD_WIDTH), ViewWindow.maxX - ViewWindow.minX);
    planeToScreenRatioY = fast_div(INT_TO_FIXED(GFX_LCD_HEIGHT), ViewWindow.maxY - ViewWindow.minY);
    minimumPlaneToScreenRatio = fast_min(planeToScreenRatioX, planeToScreenRatioY);
}

void gradient(fast_int (*function)(fast_int, fast_int), fast_int x, fast_int y, fast_int *grad_x, fast_int *grad_y) {
    fast_int divisor = INT_TO_FIXED(2);
    *grad_x = fast_div(function(x + epsilon, y) - function(x - epsilon, y), divisor);
    *grad_y = fast_div(function(x, y + epsilon) - function(x, y + epsilon), divisor);
}
fast_int lipschitzConstant(fast_int (*function)(fast_int, fast_int), fast_int x_min, fast_int x_max, fast_int y_min, fast_int y_max, bool *shouldStop) {
    fast_int max_grad = 0;
    const int SAMPLES = 2;

    for (int i = 0; i < SAMPLES; i++) {
        for (int j = 0; j < SAMPLES; j++) {
            fast_int x = x_min + FIXED_DIV(x_max - x_min, INT_TO_FIXED(SAMPLES - 1)) * i;
            fast_int y = y_min + FIXED_DIV(y_max - y_min, INT_TO_FIXED(SAMPLES - 1)) * j;

            fast_int grad_x, grad_y;
            gradient(function, x, y, &grad_x, &grad_y);

            fast_int grad_magnitude = fast_sqrt(fast_mul(grad_x, grad_x) + fast_mul(grad_y, grad_y));

            if(grad_magnitude < GRADIANT_CUTOFF_VALUE) {
                *shouldStop = true;
                return 0;
            }

            if (grad_magnitude > max_grad) {
                max_grad = grad_magnitude;
            }
        }
    }

    return max_grad;
}

void processChildOfRegion(fast_int x, fast_int y, fast_int size, uint8_t depth, fast_int (*function)(fast_int, fast_int)) {
    fast_int halfSize = fast_div(size, INT_TO_FIXED(2));
    uint8_t nextDepth = depth + 1;

    processRegion(x, y, halfSize, nextDepth, function);
    processRegion(x + halfSize, y, halfSize, nextDepth, function);
    processRegion(x, y + halfSize, halfSize, nextDepth, function);
    processRegion(x + halfSize, y + halfSize, halfSize, nextDepth, function);
}

void processRegion(fast_int x, fast_int y, fast_int size, uint8_t depth, fast_int (*function)(fast_int, fast_int)) {
    if(depth >= MAX_DEPTH) {
        //render region
        if(depth == 0) return; // should have contour line unless depth is 0

        fast_int x1 = x;
        fast_int y1 = y;
        fast_int x2 = x + size;
        fast_int y2 = y + size;

        fast_int f1 = function(x1, y1);
        fast_int f2 = function(x2, y1);
        fast_int f3 = function(x1, y2);
        fast_int f4 = function(x2, y2);

        renderRegion(function, x, y, size, f1, f2, f3, f4);
        return;
    }

    if(depth < 3) {
        // not need to calculate anything for large regions
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

    fast_int x1 = x;
    fast_int y1 = y;
    fast_int x2 = x + size;
    fast_int y2 = y + size;

    fast_int f1 = function(x1, y1);
    fast_int f2 = function(x2, y1);
    fast_int f3 = function(x1, y2);
    fast_int f4 = function(x2, y2);

    fast_int sampledMin = fast_min(fast_min(f1, f2), fast_min(f3, f4));
    fast_int sampledMax = fast_max(fast_max(f1, f2), fast_max(f3, f4));

    if(sampledMin <= 0 && sampledMax >= 0) {
        // region has contour line
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

    bool shouldStop = false;

    fast_int L = lipschitzConstant(function, x1, x2, y1, y2, &shouldStop);

    if(shouldStop) {
        renderRegion(function, x, y, size, f1, f2, f3, f4);
        return;
    }

    fast_int regionDiameter = fast_mul(size, fast_mul(FLOAT_TO_FIXED(1.41421356237), INT_TO_FIXED(2)));

    fast_int possibleMin = sampledMin - fast_mul(L, regionDiameter);
    fast_int possibleMax = sampledMax + fast_mul(L, regionDiameter);

    if(possibleMin <= 0 && possibleMax >= 0) {
        // region has contour line
        processChildOfRegion(x, y, size, depth, function);
        return;
    }

    // has no contour line
}

fast_int interpolate(fast_int f1, fast_int f2, fast_int p1, fast_int p2) {
    if(fast_abs(f2 - f1) < epsilon) {
        return fast_div(FIXED_ADD(p1, p2), INT_TO_FIXED(2));
    }
    fast_int t = fast_div(FIXED_NEG(f1), FIXED_SUB(f2, f1));
    return FIXED_ADD(p1, fast_mul(t, FIXED_SUB(p2, p1)));
}

void renderRegion(fast_int (*function)(fast_int, fast_int), fast_int x, fast_int y, fast_int size, fast_int f1, fast_int f2, fast_int f3, fast_int f4) {
    fast_int isoLevel = 0;
    uint8_t squareIndex = 0;

    if(f1 < isoLevel) squareIndex |= 1;
    if(f2 < isoLevel) squareIndex |= 2;
    if(f3 < isoLevel) squareIndex |= 4;
    if(f4 < isoLevel) squareIndex |= 8;

    if(squareIndex == 0 || squareIndex == 15) return;

    fast_int x0 = x;
    fast_int y0 = y;
    fast_int x1 = x + size;
    fast_int y1 = y + size;

    //gfx_FillScreen(255);

    // gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), FIXED_TO_INT(fast_mul(size, planeToScreenRatioX)) + 1, FIXED_TO_INT(fast_mul(size, planeToScreenRatioY)) + 1);

    switch(squareIndex) {
        case 1: {
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            
            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(topX), planeToScreenY(y0));
            break;
        }
        case 2: {
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 3: {
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 4: {
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x0), planeToScreenY(leftY));
            break;
        }
        case 5: {
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        // case 6: {
        //     break;
        // }
        case 7: {
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 8: {
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(bottomX), planeToScreenY(y1), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        // case 9: {
        //     break;
        // }
        case 10: {
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        case 11: {
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);
            fast_int bottomX = fast_clamp(interpolate(f3, f4, x0, x1), x0, x1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(bottomX), planeToScreenY(y1));
            break;
        }
        case 12: {
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 13: {
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            fast_int rightY = fast_clamp(interpolate(f2, f4, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(topX), planeToScreenY(y0), planeToScreenX(x1), planeToScreenY(rightY));
            break;
        }
        case 14: {
            fast_int topX = fast_clamp(interpolate(f1, f2, x0, x1), x0, x1);
            fast_int leftY = fast_clamp(interpolate(f1, f3, y0, y1), y0, y1);

            gfx_Line_NoClip(planeToScreenX(x0), planeToScreenY(leftY), planeToScreenX(topX), planeToScreenY(y0));
            break;
        }
        default:
            gfx_Rectangle_NoClip(planeToScreenX(x0), planeToScreenY(y0), FIXED_TO_INT(fast_mul(size, planeToScreenRatioX)) + 1, FIXED_TO_INT(fast_mul(size, planeToScreenRatioY)) + 1);
            break;
    }
}
        

int main(void) {
    gfx_Begin();

    startGraph();

    //renderImplicitFunction(circle, 100);
    renderImplicitFunction(hyperbola, 100);
    //renderImplicitFunction(csc, 0);
    renderImplicitFunction(line, 0);

    while(!os_GetCSC());

    gfx_End();
    return 0;
}