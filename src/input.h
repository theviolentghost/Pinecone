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

typedef struct InputHandler InputHandler;
typedef struct DisplayCharacter DisplayCharacter;
typedef struct FractionCharacter FractionCharacter;
typedef struct FunctionCharacter FunctionCharacter;
typedef struct ExponentCharacter ExponentCharacter;
typedef struct ParenthesesCharacter ParenthesesCharacter;
typedef struct Bounds Bounds;

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
    CHARACTER_CHARACTER, // either a number or symbol
    FUNCTION_CHARACTER, //for functions like abs, cos, etc
    FRACTION_CHARACTER,
    EXPONENT_CHARACTER,
    PARENTHESES_CHARACTER,
} CharacterType;

struct Bounds {
    int x;
    int y;
    int width;
    int height;
};

struct DisplayCharacter {
    CharacterType type;
    union {
        char character;    
        struct FunctionCharacter* function;
        struct FractionCharacter* fraction; 
        struct ExponentCharacter* exponent;
        struct ParenthesesCharacter* parentheses;
    } data;
    Bounds bounds;
};

struct FractionCharacter {
    InputHandler* numerator;    
    InputHandler* denominator;
};

struct ExponentCharacter {
    InputHandler* exponent; 
};

struct FunctionCharacter {
    char border; // '(' or '['
    InputHandler* baseInput; //input for base if applicable like roots and logs
    FunctionName name;
    InputHandler* input;    
};

struct ParenthesesCharacter {
    bool open; // true if open, false if closed
    char type; // '(' or '['
    int selfIndex; // index of the character that represents this parenthese
    int connectingIndex; // index of the character that connects to this parentheses
};

struct InputHandler {
    DisplayCharacter* buffer;
    int scale; // scale of the diplay chars
    int capacity; //current capacity of the buffer
    int size; //current size of the buffer
    int position; //current position of the cursor
    int maxSize; 
    bool inFocus;
    InputHandler* leftHandler;
    InputHandler* rightHandler;
    InputHandler* upFocus; // which handler to focus on when up is pressed
    InputHandler* downFocus; // which handler to focus on when down is pressed or enter is pressed
    Bounds bounds;
    Bounds window;
};

char keyToChar(uint8_t key);
void freeInputHandler(InputHandler* handler);
void freeDisplayCharacter(DisplayCharacter* character);
DisplayCharacter* createDisplayCharacter(CharacterType type);
InputHandler* createInputHandler();
DisplayCharacter* createCharacter(char character);
DisplayCharacter* createFractionCharacter();
DisplayCharacter* createPlaceHolderCharacter();
void renderStringAt(const char* string, int x, int y, int scale);
void renderCharAt(char character, int x, int y, int scale);
void renderDisplayCharacter(DisplayCharacter* node, int scale, int offsetX, int offsetY);
void renderInputHandler(InputHandler* handler, int offsetX, int offsetY);
void Character_render(DisplayCharacter* node, int scale, int offsetX, int offsetY);
void Fraction_render(DisplayCharacter* node, int scale, int offsetX, int offsetY);
void setDisplayCharacterPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY);
void setInputHandlerPosition(InputHandler* handler, int scale, int originY, int offsetX, int offsetY);
int getDisplayCharacterBelowOriginHeight(DisplayCharacter* node, int scale);
int getDisplayCharacterAboveOriginHeight(DisplayCharacter* node, int scale);
int getDisplayCharacterWidth(DisplayCharacter* node);
int getDisplayCharacterHeight(DisplayCharacter* node, int scale);
int getInputHandlerBelowOriginHeight(InputHandler* handler, int scale);
int getInputHandlerAboveOriginHeight(InputHandler* handler, int scale);
int getInputHandlerWidth(InputHandler* handler);
int getInputHandlerHeight(InputHandler* handler, int scale);
void Character_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY);
void Fraction_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY);
void recordInput(InputHandler* handler);
void appendDisplayCharacter(InputHandler* handler, DisplayCharacter* character);
void setDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character);
void renderInput(InputHandler* mainHandler, InputHandler* focusHandler);
void renderCursor(InputHandler* mainHandler, InputHandler* handler, int offsetX, int offsetY, bool nullCursor);
void addDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character);
int Input_moveLeft(InputHandler* handler);
int Input_moveRight(InputHandler* handler);
void Input_shiftElementsLeft(InputHandler* handler, int position);
void Input_shiftElementsRight(InputHandler* handler, int position);
bool manageSpecialKeyStates(uint8_t key);
void Placeholder_render(DisplayCharacter* node, int scale, int offsetX, int offsetY);
DisplayCharacter* createGapCharacter();
void deleteCharacterAtPosition(InputHandler* handler, int position, bool force);
void insertDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character);
void Exponent_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY);
void Exponent_render(DisplayCharacter* node, int scale, int offsetX, int offsetY);
DisplayCharacter* createFunctionCharacter(FunctionName name);
void Function_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY);
void Function_render(DisplayCharacter* node, int scale, int offsetX, int offsetY);
bool functionHasNegativeOne(FunctionName name);
bool functionHasBase(FunctionName name);
int Function_getNameWidth(DisplayCharacter* node);
char* Function_getName(DisplayCharacter* node);
int functionBorderWidth(DisplayCharacter node);
FunctionName characterToFunctionCharacter (char character);
bool functionRequiresSpecialRendering(FunctionName name);
int getStringWidth(const char* string);

#endif