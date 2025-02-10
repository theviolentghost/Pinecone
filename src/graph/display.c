#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>

#include <ti/getcsc.h> 

#define MAX_DEPTH 8
#define MIN_DEPTH 3
#define COSINE_STRAIGHT_CONTOUR_THRESHOLD 0.945f
#define epsilon 0.0001f

bool drawAxesTicks = true;
uint8_t axesColor = 0;
bool drawAxes = true;
uint8_t axesTicksColor = 0;

typedef float (*ImplicitFunction)(float x, float y);
typedef enum {
    TOP_EDGE = 0,
    BOTTOM_EDGE = 1,
    LEFT_EDGE = 2,
    RIGHT_EDGE = 3
} SquareEdge;
typedef enum {
    NORMAL = 0,
    TOP_TO_LEFT_EDGE__BOTTOM_TO_RIGHT_EDGE = 1,
    TOP_TO_RIGHT_EDGE__BOTTOM_TO_LEFT_EDGE = 2
} ContourCase;

float circle(float x, float y) {
    return x * x + y * y - 5; // Circle of radius 1
}
float hyperbola(float x, float y) {
    return x * x - y * y - 1; // Hyperbola
}
float line(float x, float y) {
    return x - y; 
}
float csc(float x, float y) {
    return x - 1/sinf(y); 
}
float gsin(float x, float y) {
    return y - sinf(x);
}
float gtan(float x, float y) {
    return y - tanf(x);
}
float inverse(float x, float y) {
    return x - 1/y;
}

struct {
    float minX, minY, maxX, maxY;
    float planeToScreenRatioX, planeToScreenRatioY, minimumPlaneToScreenRatio;
} ViewWindow = {
    .minX = -7.0f,
    .minY = -7.0f,
    .maxX = 7.0f,
    .maxY = 7.0f
};
float spareContourX0, spareContourY0, spareContourX1, spareContourY1;

int planeToScreenX(float x) {
    return (int)((x - ViewWindow.minX) * ViewWindow.planeToScreenRatioX);
}
int planeToScreenY(float y) {
    return (int)(GFX_LCD_HEIGHT - ((y - ViewWindow.minY) * ViewWindow.planeToScreenRatioY)); // flipped to account for math and computer science coordinate differences
}

void processRegion(ImplicitFunction function, float x, float y, float size, uint8_t depth);
void processChildOfRegion(ImplicitFunction function, float x, float y, float size, uint8_t depth);
void renderContour(int x1, int y1, int x2, int y2, ContourCase contourCase);
uint8_t calculateSquareIndex(float f1, float f2, float f3, float f4);
ContourCase calculateContourCase(uint8_t squareIndex);
void calculateContourWithLinearInterpolation(float x, float y, float size, uint8_t squareIndex, float f1, float f2, float f3, float f4, float *contourX0, float *contourY0, float *contourX1, float *contourY1);
void calculateContourWithNewtonMethod(ImplicitFunction function, float x, float y, float size, uint8_t squareIndex, float initContourX0, float initContourY0, float initContourX1, float initContourY1, float *contourX0, float *contourY0, float *contourX1, float *contourY1);
float calculateLipschitzConstant(ImplicitFunction function, float minX, float maxX, float minY, float maxY, uint8_t samples);
bool isContourStraight(ImplicitFunction function, float contourX0, float contourY0, float contourX1, float contourY1);
void gradient(ImplicitFunction function, float x, float y, float *gradientX, float *gradientY);
void renderRegion(ImplicitFunction function, float x, float y, float size);
float minimumFunctionValue(float f1, float f2, float f3, float f4);
float maximumFunctionValue(float f1, float f2, float f3, float f4);
void startGraph(void);
void quadTrees(ImplicitFunction function, uint8_t color);
void marchingSquares(ImplicitFunction function, uint8_t color, int cellSize);

void renderAxes() {
    gfx_SetColor(axesColor);
    gfx_HorizLine(0, planeToScreenY(0), GFX_LCD_WIDTH);
    gfx_VertLine(planeToScreenX(0), 0, GFX_LCD_HEIGHT);
}
void renderAxesTicks() {
    gfx_SetColor(axesTicksColor);
    for(int i = (int)ViewWindow.minX; i <= (int)ViewWindow.maxX; i++) {
        if(i == 0) continue;
        int x = planeToScreenX(i);
        gfx_VertLine(x, planeToScreenY(0) - 2, 4);
    }
    for(int i = (int)ViewWindow.minY; i <= (int)ViewWindow.maxY; i++) {
        if(i == 0) continue;
        int y = planeToScreenY(i);
        gfx_HorizLine(planeToScreenX(0), y - 2, 4);
    }
}
void startGraph(void) {
    ViewWindow.planeToScreenRatioX = (float)GFX_LCD_WIDTH / (ViewWindow.maxX - ViewWindow.minX);
    ViewWindow.planeToScreenRatioY = (float)GFX_LCD_HEIGHT / (ViewWindow.maxY - ViewWindow.minY);
    ViewWindow.minimumPlaneToScreenRatio = fminf(ViewWindow.planeToScreenRatioX, ViewWindow.planeToScreenRatioY);

    if(drawAxes) { renderAxes(); }
    if(drawAxesTicks) { renderAxesTicks(); }
}
void quadTrees(ImplicitFunction function, uint8_t color) {
    gfx_SetColor(color);
    processRegion(function, ViewWindow.minX, ViewWindow.minY, fmaxf(ViewWindow.maxX - ViewWindow.minX, ViewWindow.maxY - ViewWindow.minY), 0);
}

//////
//////
//////

float calculateLipschitzConstant(ImplicitFunction function, float minX, float maxX, float minY, float maxY, uint8_t samples) {
    float maxGradient = 0.0f;

    for(int i = 0; i < samples; i++) {
        for(int j = 0; j < samples; j++) {
            float x = minX + (maxX - minX) * i / (samples - 1);
            float y = minY + (maxY - minY) * j / (samples - 1);

            float gradietnX, gradientY;
            gradient(function, x, y, &gradietnX, &gradientY);

            float gradientMagnitude = sqrtf(gradietnX * gradietnX + gradientY * gradientY);
            if(gradientMagnitude > maxGradient) maxGradient = gradientMagnitude;
        }
    }

    return maxGradient;
}

bool isContourStraight(ImplicitFunction function, float contourX0, float contourY0, float contourX1, float contourY1) {
    float startContourX, startContourY;
    float endContourX, endContourY;

    gradient(function, contourX0, contourY0, &startContourX, &startContourY);
    gradient(function, contourX1, contourY1, &endContourX, &endContourY);

    float dot = startContourX * endContourX + startContourY * endContourY;

    float startGradientMagnitudeSquared = startContourX * startContourX + startContourY * startContourY;
    float endGradientMagnitudeSquared = endContourX * endContourX + endContourY * endContourY;

    float cosineApproximation = dot * dot / (startGradientMagnitudeSquared * endGradientMagnitudeSquared);

    return cosineApproximation > COSINE_STRAIGHT_CONTOUR_THRESHOLD * COSINE_STRAIGHT_CONTOUR_THRESHOLD;
}

float interpolate(float f1, float f2, float p1, float p2) {
    if (fabsf(f2 - f1) < 0.000001f) {
        // If function values are very close, return midpoint
        return (p1 + p2) * 0.5f;
    }
    float t = -f1 / (f2 - f1); // Assuming we are looking for the zero crossing (isoLevel = 0)
    return p1 + t * (p2 - p1);
}
const int MAX_NEWTON_ITERATIONS = 3; 
float newton(ImplicitFunction function, float initialGuess, float minBound, float maxBound, float fixedBound, SquareEdge side) {
    // intial guess is a point on the edge
    // fixedBound is a fixed paramater for the edge i.e. for TOP_EDGE the fixedBound would be the y of the edge (y0)
    // side being which side we are looking at

    float result = initialGuess;

    switch(side) {
        case TOP_EDGE: 
        case BOTTOM_EDGE: 
            {
                for(int i = 0; i < MAX_NEWTON_ITERATIONS; i++) {
                    float value = function(result, fixedBound);
                    float gradient = (function(result + epsilon, fixedBound) - value) / epsilon;
                    if(gradient == 0) return result;

                    result -= value / gradient;
                }
                return fmaxf(minBound, fminf(result, maxBound)); //clamped result
            }
        case LEFT_EDGE:
        case RIGHT_EDGE:
            {
                for(int i = 0; i < MAX_NEWTON_ITERATIONS; i++) {
                    float value = function(fixedBound, result);
                    float gradient = (function(fixedBound, result + epsilon) - value) / epsilon;
                    if(gradient == 0) return result;

                    result -= value / gradient;
                }
                return fmaxf(minBound, fminf(result, maxBound));
            }
    }

    return result; // should never reach here
}

void calculateContourWithLinearInterpolation(float x, float y, float size, uint8_t squareIndex, float f1, float f2, float f3, float f4, float *contourX0, float *contourY0, float *contourX1, float *contourY1) {
    float x0 = x;
    float y0 = y;
    float x1 = x + size;
    float y1 = y + size;

    switch(squareIndex) {
        case 1:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = x0;
            *contourY1 = interpolate(f1, f3, y0, y1);
            break;
        case 2:
            *contourX0 = x1;
            *contourY0 = interpolate(f2, f4, y0, y1);
            *contourX1 = interpolate(f1, f2, x0, x1);
            *contourY1 = y0;
            break;
        case 3:
            *contourX0 = x0;
            *contourY0 = interpolate(f1, f3, y0, y1);
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);
            break;
        case 4:
            *contourX0 = x0;
            *contourY0 = interpolate(f1, f3, y0, y1);
            *contourX1 = interpolate(f3, f4, x0, x1);
            *contourY1 = y1;
            break;
        case 5:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = interpolate(f3, f4, x0, x1);
            *contourY1 = y1;
            break;
        case 6:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);

            spareContourX0 = interpolate(f1, f3, x0, x1);
            spareContourY0 = y1;
            spareContourX1 = x0;
            spareContourY1 = interpolate(f1, f3, y0, y1);
            break;
        case 7:
            *contourX0 = interpolate(f3, f4, x0, x1);
            *contourY0 = y1;
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);
            break;
        case 8:
            *contourX0 = interpolate(f3, f4, x0, x1);
            *contourY0 = y1;
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);
            break;
        case 9:
            *contourX0 = x0;
            *contourY0 = interpolate(f1, f3, y0, y1);
            *contourX1 = interpolate(f1, f2, x0, x1);
            *contourY1 = y0;

            spareContourX0 = x1;
            spareContourY0 = interpolate(f2, f4, y0, y1);
            spareContourX1 = interpolate(f3, f4, x0, x1);
            spareContourY1 = y1;
            break;
        case 10:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = interpolate(f3, f4, x0, x1);
            *contourY1 = y1;
            break;
        case 11:
            *contourX0 = x0;
            *contourY0 = interpolate(f1, f3, y0, y1);
            *contourX1 = interpolate(f3, f4, x0, x1);
            *contourY1 = y1;
            break;
        case 12:
            *contourX0 = x0;
            *contourY0 = interpolate(f1, f3, y0, y1);
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);
            break;
        case 13:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = x1;
            *contourY1 = interpolate(f2, f4, y0, y1);
            break;
        case 14:
            *contourX0 = interpolate(f1, f2, x0, x1);
            *contourY0 = y0;
            *contourX1 = x0;
            *contourY1 = interpolate(f1, f3, y0, y1);
            break;
        default:
            // set to top left to bottom right
            *contourX0 = x0;
            *contourY0 = y0;
            *contourX1 = x1;
            *contourY1 = y1;
    }
}
void calculateContourWithNewtonMethod(ImplicitFunction function, float x, float y, float size, uint8_t squareIndex, float initContourX0, float initContourY0, float initContourX1, float initContourY1, float *contourX0, float *contourY0, float *contourX1, float *contourY1) {
    float x0 = x;
    float y0 = y;
    float x1 = x + size;
    float y1 = y + size;

    switch (squareIndex) {
        case 1:
            // Case 1: Only f1 (top-left) is below the isoLevel
            // line crosses top and left edge
            *contourX0 = newton(function, initContourX0, x0, x1, y0, TOP_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x0, LEFT_EDGE);
            break;
        case 2:
            // Case 2: Only f2 (top-right) is below the isoLevel
            // line crosses top and right edge
            *contourX0 = initContourX0;
            *contourY0 = newton(function, initContourY0, y0, y1, x1, RIGHT_EDGE);
            *contourX1 = newton(function, initContourX1, x0, x1, y0, TOP_EDGE);
            *contourY1 = initContourY1;
            break;
        case 3:
            // Case 3: f1 and f2 are below the isoLevel
            // line crosses left and right edge
            *contourX0 = initContourX0;
            *contourY0 = newton(function, initContourY0, y0, y1, x0, LEFT_EDGE);
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x1, RIGHT_EDGE);
            break;
        case 4:
            // Case 4: Only f3 (bottom-right) is below the isoLevel
            // line crosses bottom and right edge
            *contourX0 = initContourX0;
            *contourY0 = newton(function, initContourY0, y0, y1, x0, LEFT_EDGE);
            *contourX1 = newton(function, initContourX1, x0, x1, y1, BOTTOM_EDGE);
            *contourY1 = initContourY1;
            break;
        case 5:
            // Case 5: f1 and f3 are below the isoLevel
            // line crosses top and bottom edge
            *contourX0 = newton(function, initContourX0, x0, x1, y0, TOP_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = newton(function, initContourX1, x0, x1, y1, BOTTOM_EDGE);
            *contourY1 = initContourY1;
        case 6:
            // Case 6: f2 and f3 are below the isoLevel
            // line crosses left and bottom edge
            // special case
            break;
        case 7: // sus
            // Case 7: f1, f2, and f3 are below the isoLevel
            // line crosses left and bottom edge
            *contourX0 = newton(function, initContourX0, x0, x1, y1, BOTTOM_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x1, RIGHT_EDGE);
            break;
        case 8:
            // Case 8: Only f4 (bottom-left) is below the isoLevel
            // line crosses bottom and left edge
            *contourX0 = newton(function, initContourX0, x0, x1, y1, BOTTOM_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x1, RIGHT_EDGE);
            break;
        case 9:
            // Case 9: f1 and f4 are below the isoLevel
            // line crosses top and left edge
            // special case
            break;
        case 10:
            // Case 10: f2 and f4 are below the isoLevel
            // line crosses top and right edge
            *contourX0 = newton(function, initContourX0, x0, x1, y0, TOP_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = newton(function, initContourX1, x0, x1, y1, BOTTOM_EDGE);
            *contourY1 = initContourY1;
            break;
        case 11:
            // Case 11: f1, f2, and f4 are below the isoLevel
            // line crosses top and right edge
            *contourX0 = initContourX0;
            *contourY0 = newton(function, initContourY0, y0, y1, x0, LEFT_EDGE);
            *contourX1 = newton(function, initContourX1, x0, x1, y1, TOP_EDGE);
            *contourY1 = initContourY1;
            break;
        case 12:
            // Case 12: f3 and f4 are below the isoLevel
            // line crosses left and bottom edge
            *contourX0 = initContourX0;
            *contourY0 = newton(function, initContourY0, y0, y1, x0, LEFT_EDGE);
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x1, BOTTOM_EDGE);
            break;
        case 13:
            // Case 13: f1, f3, and f4 are below the isoLevel
            // line crosses left and bottom edge
            *contourX0 = newton(function, initContourX0, x0, x1, y0, TOP_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x1, RIGHT_EDGE);
            break;
        case 14:
            // Case 14: f2, f3, and f4 are below the isoLevel
            // line crosses top and right edge
            *contourX0 = newton(function, initContourX0, x0, x1, y0, TOP_EDGE);
            *contourY0 = initContourY0;
            *contourX1 = initContourX1;
            *contourY1 = newton(function, initContourY1, y0, y1, x0, LEFT_EDGE);
            break;
    }
}

void gradient(ImplicitFunction function, float x, float y, float *gradientX, float *gradientY) {
    *gradientX = (function(x + epsilon, y) - function(x - epsilon, y)) / (2 * epsilon);
    *gradientY = (function(x, y + epsilon) - function(x, y - epsilon)) / (2 * epsilon);
}

void renderContour(int x1, int y1, int x2, int y2, ContourCase contourCase) {
    // clamp to display
    x1 = fmaxf(0, fminf(GFX_LCD_WIDTH - 1, x1));
    y1 = fmaxf(0, fminf(GFX_LCD_HEIGHT - 1, y1));
    x2 = fmaxf(0, fminf(GFX_LCD_WIDTH - 1, x2));
    y2 = fmaxf(0, fminf(GFX_LCD_HEIGHT - 1, y2));

    if(contourCase == NORMAL) {
        gfx_Line_NoClip(x1,y1,x2, y2);
    }
    else if(contourCase == TOP_TO_LEFT_EDGE__BOTTOM_TO_RIGHT_EDGE || contourCase == TOP_TO_RIGHT_EDGE__BOTTOM_TO_LEFT_EDGE) {
        gfx_Line_NoClip(x1,y1,x2, y2);
        gfx_Line_NoClip(spareContourX0, spareContourY0, spareContourX1, spareContourY1);
    }
}

uint8_t calculateSquareIndex(float f1, float f2, float f3, float f4) {
    uint8_t squareIndex = 0;

    if(f1 < 0) squareIndex |= 1;
    if(f2 < 0) squareIndex |= 2;
    if(f3 < 0) squareIndex |= 4;
    if(f4 < 0) squareIndex |= 8;

    return squareIndex;
}
ContourCase calculateContourCase(uint8_t squareIndex) {
    switch(squareIndex) {
        case 6:
            return TOP_TO_RIGHT_EDGE__BOTTOM_TO_LEFT_EDGE;
        case 9:
            return TOP_TO_LEFT_EDGE__BOTTOM_TO_RIGHT_EDGE;
        default:
            return NORMAL;
    }
}

float minimumFunctionValue(float f1, float f2, float f3, float f4) {
    return fminf(fminf(f1, f2), fminf(f3, f4));
}

float maximumFunctionValue(float f1, float f2, float f3, float f4) {
    return fmaxf(fmaxf(f1, f2), fmaxf(f3, f4));
}

void processChildOfRegion(ImplicitFunction function, float x, float y, float size, uint8_t depth) {
    float halfSize = size / 2;
    processRegion(function, x, y, halfSize, depth + 1);
    processRegion(function, x + halfSize, y, halfSize, depth + 1);
    processRegion(function, x, y + halfSize, halfSize, depth + 1);
    processRegion(function, x + halfSize, y + halfSize, halfSize, depth + 1);
}

void processRegion(ImplicitFunction function, float x, float y, float size, uint8_t depth) {
    if(depth < MIN_DEPTH) {
        processChildOfRegion(function, x, y, size, depth);
        return;
    } 
    
    if(depth >= MAX_DEPTH) {
        renderRegion(function, x, y, size);
        return;
    }

    float f1 = function(x, y);
    float f2 = function(x + size, y);
    float f3 = function(x, y + size);
    float f4 = function(x + size, y + size);

    float sampledMinimum = minimumFunctionValue(f1, f2, f3, f4);
    float sampledMaximum = maximumFunctionValue(f1, f2, f3, f4);

    if(sampledMinimum > 0 || sampledMaximum < 0) {
        // no contour suspected within region
        // use lipschitz constant to determine if region is flat and if has a contour

        uint8_t samples = depth < 6 ? 2 : 1;
        float lipschitzConstant = calculateLipschitzConstant(function, x, x + size, y, y + size, samples);

        float regionDiameter = size * 2; // for simplicity for now

        float possibleMinimum = sampledMinimum - lipschitzConstant * regionDiameter;
        float possibleMaximum = sampledMaximum + lipschitzConstant * regionDiameter;

        if(possibleMinimum > 0 || possibleMaximum < 0) return; // no contour
    }
    else {
        // contour gauranteed (for continious fucntions)
        // determine if contour is straight for early break

        uint8_t squareIndex = calculateSquareIndex(f1, f2, f3, f4);
        if(squareIndex == 0 || squareIndex == 15) return; // no contour

        float contourX0, contourY0, contourX1, contourY1;
        ContourCase contourCase = calculateContourCase(squareIndex);

        calculateContourWithLinearInterpolation(x, y, size, squareIndex, f1, f2, f3, f4, &contourX0, &contourY0, &contourX1, &contourY1); // a rough contour placement

        if(isContourStraight(function, contourX0, contourY0, contourX1, contourY1)) {
            calculateContourWithNewtonMethod(function, x, y, size, squareIndex, contourX0, contourY0, contourX1, contourY1, &contourX0, &contourY0, &contourX1, &contourY1); // use netwon method for a more refined contour placement
            
            renderContour(planeToScreenX(contourX0), planeToScreenY(contourY0), planeToScreenX(contourX1), planeToScreenY(contourY1), contourCase);
            return;
        }
    }

    // contour suspected
    processChildOfRegion(function, x, y, size, depth);

    return;
}

void renderRegion(ImplicitFunction function, float x, float y, float size) {
    float f1 = function(x, y);
    float f2 = function(x + size, y);
    float f3 = function(x, y + size);
    float f4 = function(x + size, y + size);

    uint8_t squareIndex = calculateSquareIndex(f1, f2, f3, f4);
    if(squareIndex == 0 || squareIndex == 15) return; // no contour

    float contourX0, contourY0, contourX1, contourY1;
    ContourCase contourCase = calculateContourCase(squareIndex);

    calculateContourWithLinearInterpolation(x, y, size, squareIndex, f1, f2, f3, f4, &contourX0, &contourY0, &contourX1, &contourY1); 
    
    renderContour(planeToScreenX(contourX0), planeToScreenY(contourY0), planeToScreenX(contourX1), planeToScreenY(contourY1), contourCase);
}

void marchingSquares(ImplicitFunction function, uint8_t color, int cellSize) {
    gfx_SetColor(color);

    int numberOfRows = (int)(GFX_LCD_HEIGHT / cellSize) + 1;
    int numberOfColumns = (int)(GFX_LCD_WIDTH / cellSize) + 1;

    float deltaX = (ViewWindow.maxX - ViewWindow.minX) / numberOfColumns;
    float deltaY = (ViewWindow.maxY - ViewWindow.minY) / numberOfRows;

    float *previousColumn = malloc((numberOfRows + 1) * sizeof(float));
    float *currentColumn = malloc((numberOfRows + 1) * sizeof(float));

    for(int row = 0; row <= numberOfRows; row++) {
        float yPosition = ViewWindow.minY + row * deltaY;
        previousColumn[row] = function(ViewWindow.minX, yPosition);
    }

    for(int column = 0; column < numberOfColumns; column++) {
        float xPosition = ViewWindow.minX + column * deltaX;
        float xNextPosition = xPosition + deltaX;

        for(int row = 0; row <= numberOfRows; row++) {
            float yPosition = ViewWindow.minY + row * deltaY;
            currentColumn[row] = function(xNextPosition, yPosition);
        }

        for(int row = 0; row < numberOfRows; row++) {
            float valueTopLeft = previousColumn[row];
            float valueTopRight = currentColumn[row];
            float valueBottomLeft = previousColumn[row + 1];
            float valueBottomRight = currentColumn[row + 1];

            uint8_t squareIndex = calculateSquareIndex(valueTopLeft, valueTopRight, valueBottomLeft, valueBottomRight);
            if(squareIndex == 0 || squareIndex == 15) continue;

            float contourX0, contourY0, contourX1, contourY1;
            ContourCase contourCase = calculateContourCase(squareIndex);
            calculateContourWithLinearInterpolation(xPosition, ViewWindow.minY + row * deltaY, deltaX, squareIndex, valueTopLeft, valueTopRight, valueBottomLeft, valueBottomRight, &contourX0, &contourY0, &contourX1, &contourY1);

            renderContour(
                planeToScreenX(contourX0), planeToScreenY(contourY0),
                planeToScreenX(contourX1), planeToScreenY(contourY1),
                contourCase
            );
        }

        float *temp = previousColumn;
        previousColumn = currentColumn;
        currentColumn = temp;
    }

    free(previousColumn);
    free(currentColumn);
}











/*int main(void) {
    gfx_Begin();

    startGraph();

    quadTrees(inverse, 100);
    marchingSquares(inverse, 0, 4);
    // quadTrees(circle, 100);
    // quadTrees(hyperbola, 100);
    // marchingSquares(csc, 0, 4);
    // quadTrees(line, 0);
    // quadTrees(gsin, 100);
    // marchingSquares(gtan, 0, 4);

    while(!os_GetCSC());

    gfx_End();
    return 0;
}*/