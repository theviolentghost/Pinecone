#include "global.h"
#include <ti/getcsc.h>


const uint8_t inputTextColor = 0;
const uint8_t inputBackgroundColor = 203; //223 
const uint8_t backgroundColor = 0;

const char version[10] = "V:1.0.0";

// togglables
bool preferARCsyntax = false; // arcsin(x) > sin^-1(x)
bool preferDMSsyntax = true; // 12°20'42" > 12.345°
bool radianMode = true; // rads > degress