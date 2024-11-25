#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>
#include <keypadc.h>
#include <ti/getcsc.h>

#ifndef INPUT_H
#define INPUT_H

typedef struct Input_handler Input_handler;
typedef struct Display_char Display_char;
typedef struct Fraction_char Fraction_char;
typedef struct Function_char Function_char;
typedef struct Exponent_char Exponent_char;
typedef struct Bounds_char Bounds_char;

typedef enum {
    SIN_FUNCTION,
    COS_FUNCTION,
    TAN_FUNCTION,
    SEC_FUNCTION,
    CSC_FUNCTION,
    COT_FUNCTION,
    INVSIN_FUNCTION,
    INVCOS_FUNCTION,
    INVTAN_FUNCTION,
    INVSEC_FUNCTION,
    INVCSC_FUNCTION,
    INVCOT_FUNCTION,
    SQRT_FUNCTION,
    ROOT_FUNCTION,
    ABSOLUTE_FUNCTION,
    LOG_FUNCTION,
    LOG_BASE_FUNCTION,
    LN_FUNCTION,
} FunctionName;

typedef enum {
    EMPTY_CHARACTER,
    GAP_CHARACTER, //character inbetween fraction + exponents to allow for easy inserting
    PLACEHOLDER_CHARACTER, //the char you will see when fraction numerator/denominator/exponent are empty
    CURSOR_CHARACTER,
    VARIABLE_CHARACTER,     
    FUNCTION_CHARACTER, //for functions like abs, cos, etc
    FRACTION_CHARACTER,
    EXPONENT_CHARACTER,
} CharacterType;

struct Bounds_char {
    int x;
    int y;
    int width;
    int height;
};

struct Function_char {
    char border; 
    int base;
    struct Input_handler* baseInput; //input for base if applicable like roots and logs
    FunctionName name;
    struct Input_handler* input;    
};

struct Fraction_char {
    struct Input_handler* numerator;    
    struct Input_handler* denominator;
};

struct Exponent_char {
    struct Input_handler* exponent; 
};

struct Display_char {
    CharacterType type;
    uint8_t value;
    int scale;
    union {
        char variable;    
        struct Function_char* function;
        struct Fraction_char* fraction; 
        struct Exponent_char* exponent;
    } data;
    struct Display_char* left;
    struct Display_char* right;
    void (*setPosition)(Display_char*, Bounds_char*, int, int, int);
    void (*display)(Display_char*, int, int);
    int (*getAboveOriginHeight)(Display_char*);
    int (*getBelowOriginHeight)(Display_char*);
    int x;
    int y;
    int (*getWidth)(Display_char*);
    int (*getHeight)(Display_char*);
};

struct Input_handler {
    Display_char* buffer;
    int capacity; //current capacity of the buffer
    int scale;
    int size;
    int position;
    int maxSize;
    // int lastStartRenderingIndex; //used to keep track of where to start rendering - for preformance
    // int lastEndRenderingIndex; //used to keep track of where to end rendering - for preformance
    // int lastPositionIndex; //used to keep track of where the cursor was last
    Input_handler* left;
    Input_handler* right;
    Bounds_char* currentBounds;
    Bounds_char* maxBounds;
};

void initializeFonts(void);

void freeInputHandler(Input_handler* handler);
void freeCharacter(Display_char* character);

void recordInput(Input_handler* handler, int direction);
void setInputCharacterPositions(Input_handler* handler, int offsetX, int offsetY);
void renderInput(Input_handler* handler, bool clear, int offsetX, int offsetY);
void positionCursor(Input_handler* handler, int position, int offsetX, int offsetY);
void renderCursor(Input_handler* handler, int offsetX, int offsetY, bool nullCursor);
void deleteLastCursor(Input_handler* handler);
void setCursorAtPosition(Input_handler* handler, int position);
Input_handler* createInputHandler( int maxSize);
bool resizeBuffer(Input_handler* handler, int extraCapacity);
void clearInputHandler(Input_handler* handler, bool havePlaceholder);
int getExpressionHeight(Input_handler* handler);
int getExpressionAboveOriginHeight(Input_handler* handler);
int getExpressionBelowOriginHeight(Input_handler* handler);
int getExpressionWidth(Input_handler* handler);
void renderCharAt(char character, int x, int y, int scale);
void renderStringAt(const char* string, int x, int y, int scale);

void createPlaceholderChar(Display_char* element, int scale);
void createGapChar(Display_char* element, int scale);
void initializeDisplayChar(Display_char* element, int scale);
void initializeExponentPlaceholder(Display_char* element, int scale);
void initializeFractionPlaceholders(Display_char* element, int scale);
void initializeFunctionPlaceholders(Display_char* element, int scale);
bool isValidExponentPosition(Input_handler* handler);
void setupExponentRendering(Display_char* element);
void setupFractionRendering(Display_char* element);
void setupFunctionRendering(Display_char* element);
void setupExponentLinks(Input_handler* handler, Display_char* element);
void setupFractionLinks(Input_handler* handler, Display_char* element);
void setupFunctiontLinks(Input_handler* handler, Display_char* element);

void Character_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY);
void Character_render(Display_char* node, int offsetX, int offsetY);
int Character_getAboveOriginHeight(Display_char* node);
int Character_getBelowOriginHeight(Display_char* node);
int Character_getWidth(Display_char* node);
int Character_getHeight(Display_char* node);

void Fraction_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY);
void Fraction_render(Display_char* node, int offsetX, int offsetY);
int Fraction_getAboveOriginHeight(Display_char* node);
int Fraction_getBelowOriginHeight(Display_char* node);
int Fraction_getWidth(Display_char* node);
int Fraction_getHeight(Display_char* node);

void Exponent_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY);
void Exponent_render(Display_char* node, int offsetX, int offsetY);
int Exponent_getAboveOriginHeight(Display_char* node);
int Exponent_getBelowOriginHeight(Display_char* node);
int Exponent_getWidth(Display_char* node);
int Exponent_getHeight(Display_char* node);

void Function_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY);
void Function_render(Display_char* node, int offsetX, int offsetY);
int Function_getAboveOriginHeight(Display_char* node);
int Function_getBelowOriginHeight(Display_char* node);
int Function_getWidth(Display_char* node);
int Function_getHeight(Display_char* node);
int Function_getNameWidth(Display_char* node);

void Input_shiftRight(Input_handler* handler, int initialPosition);
void Input_shiftLeft(Input_handler* handler, int initialPosition);

#endif