#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>

#ifndef DISPLAY_H
#define DISPLAY_H

typedef float (*ImplicitFunction)(float x, float y);

extern bool drawAxesTicks;
extern uint8_t axesColor;
extern bool drawAxes;
extern uint8_t axesTicksColor;


#endif