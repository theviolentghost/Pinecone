#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>
#include <keypadc.h>
#include <ti/getcsc.h>
#include "input.h"
#include "global.h"
#include "font.h"

#define MAX_INPUT_SIZE 128
#define INITIAL_CAPACITY 32
#define CAPACITY_INCREASE_FACTOR 24

//todo fix jankiness with left and right movement between fractions

const char *CHARACTER_MAP = "\0\0\0\0\0\0\0\0\0\0\"WRMH\0\0?[VQLG\0\0:ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
bool secondEnabled = false;
bool alphaEnabled = false;
bool lowercaseEnabled = false;

uint8_t getKeyCode(void) {
    static uint8_t last_key;
    uint8_t only_key = 0;
    kb_Scan();
    for (uint8_t key = 1, group = 7; group; --group) {
        for (uint8_t mask = 1; mask; mask <<= 1, ++key) {
            if (kb_Data[group] & mask) {
                if (only_key) {
                    last_key = 0;
                    return 0;
                } else {
                    only_key = key;
                }
            }
        }
    }
    if (only_key == last_key) {
        return 0;
    }
    last_key = only_key;
    return only_key;
}

char keyToChar(uint8_t key) {
    if(alphaEnabled) {
        switch(key) {
            case sk_GraphVar: return Character_Fraction; //fraction
            case sk_1: return Character_Root; // root - temp
            case sk_2: return Character_LogBase; // log base - temp
            case sk_3: return Character_Absolute; // absoluet func - temp
        }
        if(CHARACTER_MAP[key]) return CHARACTER_MAP[key];
    }
    if(secondEnabled) {
        switch(key) {
            case sk_Power: return Character_PI; //PI
            case sk_Sin: return Character_InvSin; //sin^-1
            case sk_Cos: return Character_InvCos; //cos^-1
            case sk_Tan: return Character_InvTan; //tan^-1
            case sk_Square: return Character_SquareRoot;
            case sk_Log: return Character_TenPower; //10^
            case sk_Ln: return Character_EulerPower; //e^x
            case sk_Div: return Character_Euler; //e
        }
    }
    switch(key) {
        case sk_0: return '0';
        case sk_1: return '1';
        case sk_2: return '2';
        case sk_3: return '3';
        case sk_4: return '4';
        case sk_5: return '5';
        case sk_6: return '6';
        case sk_7: return '7';
        case sk_8: return '8';
        case sk_9: return '9';
        case sk_LParen: return '(';
        case sk_RParen: return ')';
        case sk_Div: return '/';
        case sk_Mul: return '*';
        case sk_Add: return '+';
        case sk_Sub: return '-';
        case sk_Left: return Character_Left;
        case sk_Right: return Character_Right;
        case sk_Up: return Character_Up;
        case sk_Down: return Character_Down;
        case sk_Del: return Character_Delete;
        case sk_Clear: return Character_Clear;
        case sk_Power: return Character_Power;
        case sk_Sin: return Character_Sin;
        case sk_Cos: return Character_Cos;
        case sk_Tan: return Character_Tan;
        case sk_Log: return Character_Log;
        case sk_Ln: return Character_Ln;
        case sk_Square: return Character_Square;
        case sk_Recip: return Character_Reciprocal;
        default:
            return '?';
    }
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// memory management

bool increaseCapacity(InputHandler* handler) {
    if (handler->capacity >= handler->maxSize) {
        return false;
    }
    
    int newCapacity = handler->capacity + CAPACITY_INCREASE_FACTOR;
    if (newCapacity > handler->maxSize) {
        if(handler->maxSize - handler->capacity < 2) return false; // should never happen, but if only allocated space for 1 more, then just return false
        newCapacity = handler->maxSize;
    }
    
    DisplayCharacter* newBuffer = (DisplayCharacter*)calloc(newCapacity, sizeof(DisplayCharacter));
    if (!newBuffer) return false;
    
    for (int i = 0; i < handler->size; i++) {
        newBuffer[i] = handler->buffer[i];
    }
    
    free(handler->buffer);
    handler->buffer = newBuffer;
    handler->capacity = newCapacity;
    
    return true;
}

bool enoughCapacity(InputHandler* handler, int extraCapacityNeeded) {
    return handler->size + extraCapacityNeeded <= handler->capacity;
}

void clearInputHandler(InputHandler* handler, bool freeBuffer) {
    if(!handler) return;

    if(freeBuffer) {
        for(int i = 0; i < handler->size; i++) {
            freeDisplayCharacter(&handler->buffer[i]);
        }
        free(handler->buffer);
    }

    handler->buffer = (DisplayCharacter*)calloc(INITIAL_CAPACITY, sizeof(DisplayCharacter));
    handler->capacity = INITIAL_CAPACITY;
    handler->size = 1;
    handler->scale = 1;
    handler->position = 0;
    handler->buffer[0] = *createPlaceHolderCharacter();
}

void freeInputHandler(InputHandler* handler) {
    if(!handler) return;

    for(int i = 0; i < handler->size; i++) {
        freeDisplayCharacter(&handler->buffer[i]);
    }

    free(handler->buffer);
    free(handler);
}

void freeDisplayCharacter(DisplayCharacter* character) {
    if(!character) return;
    // only neccesarly to free the data if it is a pointer not in handler

    if(character->type == FRACTION_CHARACTER) {
        freeInputHandler(character->data.fraction->numerator);
        freeInputHandler(character->data.fraction->denominator);
        free(character->data.fraction);
    }
    else if(character->type == EXPONENT_CHARACTER) {
        freeInputHandler(character->data.exponent);
        free(character->data.exponent);
    }
    else if(character->type == FUNCTION_CHARACTER) {
        freeInputHandler(character->data.function->input);
        if(character->data.function->baseInput) freeInputHandler(character->data.function->baseInput);
        free(character->data.function);
    }

    free(character);
}

DisplayCharacter* createDisplayCharacter(CharacterType type) {
    DisplayCharacter* character = (DisplayCharacter*)malloc(sizeof(DisplayCharacter));
    if(character) {
        character->type = type;
    }
    return character;
}

InputHandler* createInputHandler() {
    InputHandler* handler = (InputHandler*)malloc(sizeof(InputHandler));
    if(handler) {
        handler->buffer = (DisplayCharacter*)calloc(INITIAL_CAPACITY, sizeof(DisplayCharacter));
        handler->capacity = INITIAL_CAPACITY;
        handler->size = 1;
        handler->scale = 1;
        handler->position = 0;
        handler->maxSize = MAX_INPUT_SIZE;
        handler->bounds = *(Bounds*)malloc(sizeof(Bounds));
        handler->window = *(Bounds*)malloc(sizeof(Bounds));

        handler->buffer[0] = *createPlaceHolderCharacter();
    }
    return handler;
}

DisplayCharacter* createPlaceHolderCharacter() {
    return createDisplayCharacter(PLACEHOLDER_CHARACTER);
}

DisplayCharacter* createEmptyCharacter() {
    return createDisplayCharacter(EMPTY_CHARACTER);
}

DisplayCharacter* createGapCharacter() {
    return createDisplayCharacter(GAP_CHARACTER);
}

DisplayCharacter* createCharacter(char character) {
    DisplayCharacter* displayCharacter = createDisplayCharacter(CHARACTER_CHARACTER);
    if(displayCharacter) {
        displayCharacter->data.character = character;
    }
    return displayCharacter;
}

DisplayCharacter* createFractionCharacter() {
    DisplayCharacter* displayCharacter = createDisplayCharacter(FRACTION_CHARACTER);
    displayCharacter->data.fraction = (FractionCharacter*)malloc(sizeof(FractionCharacter));

    if(displayCharacter && displayCharacter->data.fraction) {
        displayCharacter->data.fraction->numerator = createInputHandler();
        displayCharacter->data.fraction->denominator = createInputHandler();

        displayCharacter->data.fraction->numerator->rightHandler = displayCharacter->data.fraction->denominator;
        displayCharacter->data.fraction->denominator->leftHandler = displayCharacter->data.fraction->numerator;

        return displayCharacter;
    }

    // failed, so free memory
    freeDisplayCharacter(displayCharacter);

    return NULL;
}

DisplayCharacter* createExponentCharacter() {
    DisplayCharacter* displayCharacter = createDisplayCharacter(EXPONENT_CHARACTER);
    displayCharacter->data.exponent = (ExponentCharacter*)malloc(sizeof(ExponentCharacter));

    if(displayCharacter && displayCharacter->data.exponent) {
        displayCharacter->data.exponent->exponent = createInputHandler();

        return displayCharacter;
    }

    // failed, so free memory
    freeDisplayCharacter(displayCharacter);

    return NULL;
}

DisplayCharacter* createFunctionCharacter(FunctionName name) {
    DisplayCharacter* displayCharacter = createDisplayCharacter(FUNCTION_CHARACTER);
    displayCharacter->data.function = (FunctionCharacter*)malloc(sizeof(FunctionCharacter));

    if(displayCharacter && displayCharacter->data.function) {
        displayCharacter->data.function->name = name;
        displayCharacter->data.function->input = createInputHandler();

        switch(name) {
            case SIN_FUNCTION:
            case COS_FUNCTION:
            case TAN_FUNCTION:
            case CSC_FUNCTION:
            case SEC_FUNCTION:
            case COT_FUNCTION:
            case INVSIN_FUNCTION:
            case INVCOS_FUNCTION:
            case INVTAN_FUNCTION:
            case INVCSC_FUNCTION:
            case INVSEC_FUNCTION:
            case INVCOT_FUNCTION:
            case LOG_FUNCTION:
            case LOG_BASE_FUNCTION:
            case LN_FUNCTION:
                displayCharacter->data.function->border = Charcater_Top_Open_Parenthesis;
                break;
            default:
                break;
        }

        if(name == ROOT_FUNCTION || name == LOG_BASE_FUNCTION) {
            displayCharacter->data.function->baseInput = createInputHandler();
        }

        return displayCharacter;
    }

    // failed, so free memory
    freeDisplayCharacter(displayCharacter);

    return NULL;
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// rendering

void renderStringAt(const char* string, int x, int y, int scale) {
    int width = 0;
    while (*string) {
        renderCharAt(*string, x + width, y, scale);
        width += gfx_GetCharWidth(*string);
        string++;
    }
}

void renderCharAt(char character, int x, int y, int scale) {
    if (x < 0 || x > (int)GFX_LCD_WIDTH - gfx_GetCharWidth(character) || y < 0 || y > (int)GFX_LCD_HEIGHT - 8 * scale) return; // out of bounds of screen

    gfx_SetTextScale(1, scale); 
    gfx_SetTextXY(x, y);
    gfx_PrintChar(character);
}

void renderDisplayCharacter(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    switch(node->type) {
        case CHARACTER_CHARACTER:
            Character_render(node, scale, offsetX, offsetY);
            break;
        case PLACEHOLDER_CHARACTER:
        //case GAP_CHARACTER:
            Placeholder_render(node, scale, offsetX, offsetY);
            break;
        case FRACTION_CHARACTER:
            Fraction_render(node, scale, offsetX, offsetY);
            break;
        case EXPONENT_CHARACTER:
            Exponent_render(node, scale, offsetX, offsetY);
            break;
        case FUNCTION_CHARACTER:
            Function_render(node, scale, offsetX, offsetY);
            break;
        default:
            break;
    }
}

void renderInputHandler(InputHandler* handler, int offsetX, int offsetY) {
    if(!handler || !handler->buffer) return;

    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if(currentChar->type != EMPTY_CHARACTER && currentChar->type != GAP_CHARACTER) {
            renderDisplayCharacter(currentChar, handler->scale, offsetX, offsetY);
        }
    }
}

void Character_render(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    gfx_SetTextFGColor(inputTextColor);
    renderCharAt(node->data.character, node->bounds.x + offsetX, node->bounds.y + offsetY, scale);
}

void Placeholder_render(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    gfx_SetTextFGColor(inputTextColor);
    renderCharAt(Character_PlaceHolder, node->bounds.x + offsetX, node->bounds.y + offsetY, scale);
}

void Fraction_render(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    int horizontalLineLength = getDisplayCharacterWidth(node); // clamp later

    gfx_SetColor(inputTextColor);
    gfx_HorizLine_NoClip(node->bounds.x + offsetX, node->bounds.y + offsetY + getDisplayCharacterAboveOriginHeight(node, scale), horizontalLineLength); 

    renderInputHandler(node->data.fraction->numerator, offsetX, offsetY);
    renderInputHandler(node->data.fraction->denominator, offsetX, offsetY);
}

void Exponent_render(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    renderInputHandler(node->data.exponent->exponent, offsetX, offsetY);
}

void Function_render(DisplayCharacter* node, int scale, int offsetX, int offsetY) {
    //render input
    renderInputHandler(node->data.function->input, offsetX, offsetY); 

    // render function name
    char* functionName = Function_getName(node);
    bool specialRendering = functionRequiresSpecialRendering(node->data.function->name);

    if(functionName && functionName[0] != '#') {
        int aboveOriginHeight = getDisplayCharacterAboveOriginHeight(node, scale);
        int textShiftY = aboveOriginHeight - (4 * scale); // center the function name vertically

        // render func name
        gfx_SetTextFGColor(inputTextColor);
        renderStringAt(functionName, node->bounds.x + offsetX, node->bounds.y + offsetY + textShiftY, scale);

        //render border
        if(node->data.function->border && !(node->data.function->name == LOG_BASE_FUNCTION)) {
            int belowOriginHeight = getDisplayCharacterBelowOriginHeight(node, scale);
            int topOffsetY = -(aboveOriginHeight - (4 * scale)) + textShiftY;
            int bottomOffsetY = belowOriginHeight - (4 * scale) + textShiftY;
            int functionNameWidth = Function_getNameWidth(node);
            int borderWidth = functionBorderWidth(*node);

            renderCharAt(node->data.function->border, node->bounds.x + offsetX + functionNameWidth, node->bounds.y + offsetY + topOffsetY, scale); // top left corner 
            renderCharAt(node->data.function->border + 1, node->bounds.x + offsetX + functionNameWidth, node->bounds.y + offsetY + bottomOffsetY, scale); // bottom left corner
            renderCharAt(node->data.function->border + 2, node->bounds.x + offsetX + node->bounds.width - borderWidth, node->bounds.y + offsetY + topOffsetY, scale); // top right corner
            renderCharAt(node->data.function->border + 3, node->bounds.x + offsetX + + node->bounds.width - borderWidth, node->bounds.y + offsetY + bottomOffsetY, scale); // bottom right corner

            int length = bottomOffsetY - topOffsetY;
            gfx_SetColor(inputTextColor);
            gfx_VertLine(node->bounds.x + offsetX + functionNameWidth + 3, node->bounds.y + offsetY + topOffsetY + (4 * scale), length); // left
            gfx_VertLine(node->bounds.x + offsetX + node->bounds.width - borderWidth + 4, node->bounds.y + offsetY + topOffsetY + (4 * scale), length); // right
        }

        if(specialRendering) {
            // should only include -1 in inverse function like sin^-1
            if(functionHasNegativeOne(node->data.function->name)) {
                int functionNameWidth = Function_getNameWidth(node);
                renderStringAt("-1", node->bounds.x + offsetX + functionNameWidth - 12, node->bounds.y + offsetY + textShiftY - (4 * scale), scale);
            }
            else if(node->data.function->name == LOG_BASE_FUNCTION) {
                // render base
                renderInputHandler(node->data.function->baseInput, offsetX, offsetY);

                int belowOriginHeight = getDisplayCharacterBelowOriginHeight(node, scale);
                int topOffsetY = -(aboveOriginHeight - (4 * scale)) + textShiftY;
                int bottomOffsetY = belowOriginHeight - (4 * scale) + textShiftY;
                int functionNameWidth = Function_getNameWidth(node);
                int borderWidth = functionBorderWidth(*node);

                renderCharAt(Charcater_Top_Open_Parenthesis, node->bounds.x + offsetX + functionNameWidth, node->bounds.y + offsetY + topOffsetY, scale); // top left corner 
                renderCharAt(Charcater_Top_Open_Parenthesis + 1, node->bounds.x + offsetX + functionNameWidth, node->bounds.y + offsetY + bottomOffsetY, scale); // bottom left corner
                renderCharAt(Charcater_Top_Open_Parenthesis + 2, node->bounds.x + offsetX + node->bounds.width - borderWidth, node->bounds.y + offsetY + topOffsetY, scale); // top right corner
                renderCharAt(Charcater_Top_Open_Parenthesis + 3, node->bounds.x + offsetX + + node->bounds.width - borderWidth, node->bounds.y + offsetY + bottomOffsetY, scale); // bottom right corner

                int length = bottomOffsetY - topOffsetY;
                gfx_SetColor(inputTextColor);
                gfx_VertLine(node->bounds.x + offsetX + functionNameWidth + 3, node->bounds.y + offsetY + topOffsetY + (4 * scale), length); // left
                gfx_VertLine(node->bounds.x + offsetX + node->bounds.width - borderWidth + 4, node->bounds.y + offsetY + topOffsetY + (4 * scale), length); // right

            }
        }
    }
    // no name: sqrt, root, abs
    else {
        switch (node->data.function->name) {
            case ABSOLUTE_FUNCTION:
                gfx_SetColor(inputTextColor);
                gfx_VertLine(node->bounds.x + offsetX + 3, node->bounds.y + offsetY, getDisplayCharacterHeight(node, scale)); // left
                gfx_VertLine(node->bounds.x + offsetX + node->bounds.width - 4, node->bounds.y + offsetY, getDisplayCharacterHeight(node, scale)); // right
                break;
            case SQRT_FUNCTION: {
                int nameWidth = Function_getNameWidth(node);
                int height = getDisplayCharacterHeight(node, scale);
                int inputWidth = getInputHandlerWidth(node->data.function->input);

                gfx_SetColor(inputTextColor);
                gfx_Line(node->bounds.x + offsetX + nameWidth - 6, node->bounds.y + offsetY + height - 2, node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY + height);
                gfx_VertLine(node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY, height); // side bar 
                gfx_HorizLine(node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY, inputWidth + 2); // top bar 
                break;
            }
            case ROOT_FUNCTION: {
                int nameWidth = Function_getNameWidth(node);
                int height = getDisplayCharacterHeight(node, scale);
                int inputWidth = getInputHandlerWidth(node->data.function->input);
                int baseInputWidth = getInputHandlerWidth(node->data.function->baseInput);

                renderInputHandler(node->data.function->baseInput, offsetX, offsetY); 

                gfx_SetColor(inputTextColor);
                gfx_Line(node->bounds.x + offsetX + nameWidth - 6, node->bounds.y + offsetY + height - 2, node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY + height);
                gfx_VertLine(node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY, height); // side bar 
                gfx_HorizLine(node->bounds.x + offsetX + nameWidth - 2, node->bounds.y + offsetY, inputWidth + 2); // top bar 
                break;
            }
            default:
                break;
        }
    }
}

bool functionRequiresSpecialRendering(FunctionName name) {
    // has special symbols like -1 in sin^-1 or sqrt
    switch(name) {
        case INVSIN_FUNCTION:
        case INVCOS_FUNCTION:
        case INVTAN_FUNCTION:
        case INVCSC_FUNCTION:
        case INVSEC_FUNCTION:
        case INVCOT_FUNCTION:
            return !preferARCsyntax;
        case SQRT_FUNCTION:
        case ROOT_FUNCTION:
        case ABSOLUTE_FUNCTION:
        case LOG_BASE_FUNCTION:
            return true;
        default:
            return false;
    }
}


/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////
// positioning

void setDisplayCharacterPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY) {
    switch(node->type) {
        case CHARACTER_CHARACTER:
        case PLACEHOLDER_CHARACTER:
        case EMPTY_CHARACTER:
        case GAP_CHARACTER:
            Character_setPosition(node, scale, originY, offsetX, offsetY);
            break;
        case FRACTION_CHARACTER:
            Fraction_setPosition(node, scale, originY, offsetX, offsetY);
            break;
        case EXPONENT_CHARACTER:
            Exponent_setPosition(node, scale, originY, offsetX, offsetY);
            break;
        case FUNCTION_CHARACTER:
            Function_setPosition(node, scale, originY, offsetX, offsetY);
            break;
        default:
            break;
    }
}

void setInputHandlerPosition(InputHandler* handler, int scale, int originY, int offsetX, int offsetY) {
    if(!handler || !handler->buffer) return;

    //handler->bounds.x = offsetX;
    //handler->bounds.y = offsetY + originY - getInputHandlerBelowOriginHeight(handler, scale);

    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if((currentChar->type != EMPTY_CHARACTER && currentChar->type != GAP_CHARACTER) || (index == handler->position && handler->inFocus)) {
            setDisplayCharacterPosition(currentChar, scale, originY, offsetX, offsetY);
            offsetX += getDisplayCharacterWidth(currentChar);
            offsetX += inputFieldCharacterSpacing;
        }
    }
}

int getDisplayCharacterBelowOriginHeight(DisplayCharacter* node, int scale) {
    switch (node->type) {
        case CHARACTER_CHARACTER:
        case EMPTY_CHARACTER:
        case PLACEHOLDER_CHARACTER:
        case GAP_CHARACTER:
            return getDisplayCharacterHeight(node, scale) / 2;
        case FRACTION_CHARACTER:
            return getInputHandlerHeight(node->data.fraction->denominator, scale) + 2; // acount for bar and padding
        case EXPONENT_CHARACTER:
            return (4 * scale);
        case FUNCTION_CHARACTER: 
            return getInputHandlerBelowOriginHeight(node->data.function->input, scale);
        default:
            return 0;
    }
}

int getDisplayCharacterAboveOriginHeight(DisplayCharacter* node, int scale) {
    switch (node->type) {
        case CHARACTER_CHARACTER:
        case EMPTY_CHARACTER:
        case PLACEHOLDER_CHARACTER:
        case GAP_CHARACTER:
            return getDisplayCharacterHeight(node, scale) / 2;
        case FRACTION_CHARACTER:
            return getInputHandlerHeight(node->data.fraction->numerator, scale) + 1; // account for padding
        case EXPONENT_CHARACTER:
            return getInputHandlerHeight(node->data.exponent->exponent, scale);
        case FUNCTION_CHARACTER: {
            int additionalPadding = 0;

            if(functionHasNegativeOne(node->data.function->name)) {
                additionalPadding = 4 * scale; // account for negative one
            }
            else if(node->data.function->name == SQRT_FUNCTION) {
                additionalPadding = 2 * scale; // account for bar
            }
            else if(node->data.function->name == ROOT_FUNCTION) {
                return fmax(getInputHandlerAboveOriginHeight(node->data.function->input, scale) + 1, -(2 * scale) + getInputHandlerHeight(node->data.function->baseInput, scale) + 1); // account for bar and base input
            }

            return getInputHandlerAboveOriginHeight(node->data.function->input, scale) + additionalPadding;
        }
        default:
            return 0;
    }
}

int getDisplayCharacterWidth(DisplayCharacter* node) {
    switch (node->type) {
        case CHARACTER_CHARACTER:
        case PLACEHOLDER_CHARACTER:
            return gfx_GetCharWidth(node->data.character);
        case FRACTION_CHARACTER:
            return fmax(getInputHandlerWidth(node->data.fraction->numerator), getInputHandlerWidth(node->data.fraction->denominator));
        case EXPONENT_CHARACTER:
            return getInputHandlerWidth(node->data.exponent->exponent);
        case FUNCTION_CHARACTER: {
            int functionNameWidth = Function_getNameWidth(node);
            int borderWidth = functionBorderWidth(*node);

            return functionNameWidth + (borderWidth * 2) + getInputHandlerWidth(node->data.function->input);
        }
        case EMPTY_CHARACTER:
        case GAP_CHARACTER:
            return 8;
        default:
            return 0;
    }
}

int getDisplayCharacterHeight(DisplayCharacter* node, int scale) {
    switch (node->type) {
        case CHARACTER_CHARACTER:
        case EMPTY_CHARACTER:
        case PLACEHOLDER_CHARACTER:
        case GAP_CHARACTER:
            return 8 * scale;
        case FRACTION_CHARACTER:
            return getDisplayCharacterAboveOriginHeight(node, scale) + getDisplayCharacterBelowOriginHeight(node, scale); 
        case EXPONENT_CHARACTER:
            return getInputHandlerHeight(node->data.exponent->exponent, scale) + (4 * scale);
        case FUNCTION_CHARACTER: {
            int additionalPadding = 0;

            if(functionHasNegativeOne(node->data.function->name)) {
                additionalPadding = 4 * scale - 1; // account for negative one in "(cos^-1)" etc.
            }
            else if(node->data.function->name == SQRT_FUNCTION) {
                additionalPadding = 2 * scale; // account for bar
            }
            else if(node->data.function->name == ROOT_FUNCTION) {
                return fmax(getInputHandlerHeight(node->data.function->input, scale) + 1, (4 * scale) + getInputHandlerHeight(node->data.function->baseInput, scale) + 1); // account for bar and base input
            }

            return getInputHandlerHeight(node->data.function->input, scale) + additionalPadding + 1;
        }
        default:
            return 0;
    }
}

int getInputHandlerBelowOriginHeight(InputHandler* handler, int scale) {
    if(!handler || !handler->buffer) return 0;

    int belowOriginHeight = 0;
    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if(currentChar->type != EMPTY_CHARACTER) {
            belowOriginHeight = fmax(belowOriginHeight, getDisplayCharacterBelowOriginHeight(currentChar, scale));
        }
    }
    return belowOriginHeight;
}

int getInputHandlerAboveOriginHeight(InputHandler* handler, int scale) {
    if(!handler || !handler->buffer) return 0;

    int aboveOriginHeight = 0;
    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if(currentChar->type != EMPTY_CHARACTER) {
            aboveOriginHeight = fmax(aboveOriginHeight, getDisplayCharacterAboveOriginHeight(currentChar, scale));
        }
    }
    return aboveOriginHeight;
}

int getInputHandlerWidth(InputHandler* handler) {
    if(!handler || !handler->buffer) return 0;

    int width = 0;
    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if((currentChar->type != EMPTY_CHARACTER && currentChar->type != GAP_CHARACTER) || (index == handler->position && handler->inFocus)) { // always in the case of cursor
            width += getDisplayCharacterWidth(currentChar);
            if(index < handler->size - 2) width += inputFieldCharacterSpacing; // not include spacing after last character, -2 because of empty character at end
        }
    }
    return width;
}

int getInputHandlerHeight(InputHandler* handler, int scale) {
    if(!handler || !handler->buffer) return 0;

    int height = 0;
    for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* currentChar = &handler->buffer[index];
        if(currentChar->type != EMPTY_CHARACTER) {
            height = fmax(height, getDisplayCharacterHeight(currentChar, scale));
        }
    }
    return height;
}







// Note: position should always be the furthest top left corner of the character

void Character_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY) {
    int belowOriginHeight = getDisplayCharacterBelowOriginHeight(node, scale);
    node->bounds.x = offsetX;
    node->bounds.y = offsetY + originY - belowOriginHeight; 
    node->bounds.height = belowOriginHeight * 2;
    node->bounds.width = 8;
}

void Fraction_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY) {
    node->bounds.x = offsetX;
    node->bounds.y = offsetY + originY - getDisplayCharacterAboveOriginHeight(node, scale);

    int denominatorWidth = getInputHandlerWidth(node->data.fraction->denominator);
    int numeratorWidth = getInputHandlerWidth(node->data.fraction->numerator);
    int fractionWidth = fmax(denominatorWidth, numeratorWidth);

    int numeratorHeight = getInputHandlerHeight(node->data.fraction->numerator, scale);
    int numeratorAboveOriginHeight = getInputHandlerAboveOriginHeight(node->data.fraction->numerator, scale);
    int denominatorAboveOriginHeight = getInputHandlerAboveOriginHeight(node->data.fraction->denominator, scale);

    setInputHandlerPosition(node->data.fraction->numerator, scale, originY + numeratorAboveOriginHeight - numeratorHeight - 1, offsetX + (fractionWidth - numeratorWidth) / 2, offsetY);
    setInputHandlerPosition(node->data.fraction->denominator, scale, originY + denominatorAboveOriginHeight + 2, offsetX + (fractionWidth - denominatorWidth) / 2, offsetY);

    node->bounds.width = getDisplayCharacterWidth(node);
    node->bounds.height = getDisplayCharacterHeight(node, scale);
}

void Exponent_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY) {
    int height = getDisplayCharacterHeight(node, scale);

    node->bounds.x = offsetX;
    node->bounds.y = offsetY + originY - height - 4 * scale;

    int belowOriginHeight = getInputHandlerBelowOriginHeight(node->data.exponent->exponent, scale);
    setInputHandlerPosition(node->data.exponent->exponent, scale, originY - belowOriginHeight, offsetX, offsetY);

    node->bounds.width = getDisplayCharacterWidth(node);
    node->bounds.height = getDisplayCharacterHeight(node, scale);
}

int Function_getNameWidth(DisplayCharacter* node) {
    switch(node->data.function->name) {
        case INVSIN_FUNCTION: 
        case INVCOS_FUNCTION: 
        case INVTAN_FUNCTION: 
        case INVCSC_FUNCTION: 
        case INVSEC_FUNCTION: 
        case INVCOT_FUNCTION: 
            return preferARCsyntax ? getStringWidth(Function_getName(node)) : getStringWidth(Function_getName(node)) + 6; //ammount of chars * 8 (char width) 
        case SIN_FUNCTION: 
        case COS_FUNCTION: 
        case TAN_FUNCTION: 
        case CSC_FUNCTION: 
        case SEC_FUNCTION: 
        case COT_FUNCTION: 
        case LOG_FUNCTION: 
            return getStringWidth(Function_getName(node));
        case ROOT_FUNCTION:
            return fmax(6, getInputHandlerWidth(node->data.function->baseInput)) + 4;
        case SQRT_FUNCTION:
            return 6; //root symbol
        case ABSOLUTE_FUNCTION:
            return 0;
        case LN_FUNCTION:
            return getStringWidth("ln");
        case LOG_BASE_FUNCTION:
            return getStringWidth("log") + getInputHandlerWidth(node->data.function->baseInput) + 2; //3 chars + base width
    }
    return 8; //default char width
}

int functionBorderWidth(DisplayCharacter node) {
    return node.data.function->border ? gfx_GetCharWidth(node.data.function->border) : 0;
}

bool functionHasBase(FunctionName name) {
    switch(name) {
        case ROOT_FUNCTION:
        case LOG_BASE_FUNCTION:
            return true;
        default:
            return false;
    }
}

bool functionHasNegativeOne(FunctionName name) {
    switch(name) {
        case INVSIN_FUNCTION:
        case INVCOS_FUNCTION:
        case INVTAN_FUNCTION:
        case INVCSC_FUNCTION:
        case INVSEC_FUNCTION:
        case INVCOT_FUNCTION:
            return !preferARCsyntax;
        default:
            return false;
    }
}

char* Function_getName(DisplayCharacter* node) {
    switch(node->data.function->name) {
        case SIN_FUNCTION: return "sin";
        case COS_FUNCTION: return "cos";
        case TAN_FUNCTION: return "tan";
        case CSC_FUNCTION: return "csc";
        case SEC_FUNCTION: return "sec";
        case COT_FUNCTION: return "cot";
        case INVSIN_FUNCTION: return (preferARCsyntax ? "arcsin" : "sin");
        case INVCOS_FUNCTION: return (preferARCsyntax ? "arccos" : "cos");
        case INVTAN_FUNCTION: return (preferARCsyntax ? "arctan" : "tan");
        case INVCSC_FUNCTION: return (preferARCsyntax ? "arccsc" : "csc");
        case INVSEC_FUNCTION: return (preferARCsyntax ? "arcsec" : "sec");
        case INVCOT_FUNCTION: return (preferARCsyntax ? "arccot" : "cot");
        case LOG_FUNCTION: 
        case LOG_BASE_FUNCTION: 
            return "log";
        case LN_FUNCTION: return "ln";
        default: return "#";
    }
}

int getStringWidth(const char* string) {
    int width = 0;
    while (*string) {
        width += gfx_GetCharWidth(*string);
        string++;
    }
    return width;
}

void Function_setPosition(DisplayCharacter* node, int scale, int originY, int offsetX, int offsetY) {
    node->bounds.x = offsetX;
    node->bounds.y = offsetY + originY - getDisplayCharacterAboveOriginHeight(node, scale);

    int functionNameWidth = Function_getNameWidth(node);
    int borderWidth = functionBorderWidth(*node);
    setInputHandlerPosition(node->data.function->input, scale, originY, offsetX + functionNameWidth + borderWidth, offsetY); 

    if(functionHasBase(node->data.function->name)) {
        if(node->data.function->name == ROOT_FUNCTION) {
            setInputHandlerPosition(node->data.function->baseInput, scale, originY, offsetX, offsetY - getInputHandlerBelowOriginHeight(node->data.function->baseInput, scale) + 3 * scale); 
        }
        else if(node->data.function->name == LOG_BASE_FUNCTION) {
            setInputHandlerPosition(node->data.function->baseInput, scale, originY, offsetX + getStringWidth("log") + 1, offsetY + getInputHandlerAboveOriginHeight(node->data.function->baseInput, scale)); 
        }
    }

    node->bounds.width = getDisplayCharacterWidth(node);
    node->bounds.height = getDisplayCharacterHeight(node, scale);
}

/////////////////////////////////////////
/////////////////////////////////////////
/////////////////////////////////////////

void recordInput(InputHandler* handler) {
    if (!handler || !handler->buffer) return;

    handler->inFocus = true;

    InputHandler* mainInput = handler;
    while(mainInput->leftHandler) {
        mainInput = mainInput->leftHandler;
    }

    uint8_t previousKey = 0;
    uint8_t key = 0;

    renderInput(mainInput, handler); // initial render

    do {
        key = getKeyCode();

        DisplayCharacter* elementToManipulate = &handler->buffer[handler->position];

        if (key && key != previousKey) {
            if(!enoughCapacity(handler, 4)) { // 4 is the maximum amount of characters that should be added at once
                bool increaseAllowed = increaseCapacity(handler);

                if(!increaseAllowed) {
                    return;
                }
            }

            char character = keyToChar(key);

            if(manageSpecialKeyStates(key)) {
                renderCursor(mainInput, handler, mainInput->window.width/2 - handler->buffer[handler->position].bounds.x, -(elementToManipulate->bounds.y + elementToManipulate->bounds.height/2 - mainInput->window.y - mainInput->window.height/2), false);
                continue;
            }

            switch (character) {
                case Character_Left: { // shift position left
                    int result = Input_moveLeft(handler);
                    if(result == 1) return; // exit loop
                    break;
                }
                case Character_Right: { // shift position right
                    int result = Input_moveRight(handler);
                    if(result == 1) return; // exit loop
                    break;
                }
                case Character_Delete: { // delete character at position
                    if(handler->size > 1) {
                        deleteCharacterAtPosition(handler, handler->position, false);
                    } else if(handler->buffer[0].type == PLACEHOLDER_CHARACTER && handler->leftHandler) {
                        // implying that current handler is empty so delete it and move to left handler if it exists
                        // only exception if it is a main input (does not have left handler)
                        deleteCharacterAtPosition(handler->leftHandler, handler->leftHandler->position, false); //delete fraction/exponent
                        recordInput(handler->leftHandler);
                        return; // exit loop
                    }
                    break;
                }
                case Character_Clear: { // clear all characters
                    if(handler->buffer[0].type == PLACEHOLDER_CHARACTER && handler->leftHandler) { 
                        // implying that current handler is empty so delete it and move to left handler if it exists
                        // only exception if it is a main input (does not have left handler)
                        deleteCharacterAtPosition(handler->leftHandler, handler->leftHandler->position, false); //delete fraction/exponent
                        recordInput(handler->leftHandler);
                        return; // exit loop
                    }
                    clearInputHandler(handler, false);
                    break;
                }
                case Character_Fraction: { // create fraction in place of elementToManipulate, returns numerator
                    DisplayCharacter* fraction = createFractionCharacter();
                    if(fraction) {
                        addDisplayCharacterAtPosition(handler, handler->position, createGapCharacter()); // add a gap character
                        insertDisplayCharacterAtPosition(handler, handler->position, fraction);

                        fraction->data.fraction->numerator->leftHandler = handler;
                        fraction->data.fraction->denominator->rightHandler = handler;

                        // start inputing into the numerator of fraction
                        recordInput(fraction->data.fraction->numerator);
                        return; // break out of this loop
                    }
                    break;
                }
                case Character_Power: {
                    if(handler->position == 0) break;

                    DisplayCharacter* previousCharacter = &handler->buffer[handler->position - 1];

                    if(
                        previousCharacter->type == GAP_CHARACTER ||
                        previousCharacter->type == EMPTY_CHARACTER || 
                        previousCharacter->type == PLACEHOLDER_CHARACTER || 
                        previousCharacter->type == FRACTION_CHARACTER
                    ) break;


                    DisplayCharacter* exponent = createExponentCharacter();
                    if(exponent) {
                        addDisplayCharacterAtPosition(handler, handler->position, createGapCharacter()); // add a gap character
                        insertDisplayCharacterAtPosition(handler, handler->position, exponent);

                        exponent->data.exponent->exponent->leftHandler = handler;
                        exponent->data.exponent->exponent->rightHandler = handler;
                        // start inputing into the exponent
                        recordInput(exponent->data.exponent->exponent);
                        return; // break out of this loop
                    }
                    break;
                }
                case Character_Sin:
                case Character_Cos:
                case Character_Tan:
                case Character_Csc:
                case Character_Sec:
                case Character_Cot:
                case Character_InvSin: 
                case Character_InvCos: 
                case Character_InvTan: 
                case Character_InvCsc: 
                case Character_InvSec: 
                case Character_InvCot:
                case Character_Log:
                case Character_Ln:
                case Character_SquareRoot:
                case Character_Root:
                case Character_Absolute:
                case Character_LogBase:
                {
                    DisplayCharacter* function = createFunctionCharacter((FunctionName)characterToFunctionCharacter(character));
                    if(function) {
                        addDisplayCharacterAtPosition(handler, handler->position, createGapCharacter()); // add a gap character before function
                        insertDisplayCharacterAtPosition(handler, handler->position, function);

                        function->data.function->input->leftHandler = handler;
                        function->data.function->input->rightHandler = handler;

                        if(functionHasBase(function->data.function->name)) {
                            function->data.function->baseInput->leftHandler = handler;
                            function->data.function->baseInput->rightHandler = function->data.function->input;

                            function->data.function->input->leftHandler = function->data.function->baseInput;
                        }

                        // start inputing into the function
                        recordInput(function->data.function->input);
                        return; // break out of this loop
                    }
                    break;
                }
                default: {
                    // set character at position and shift position by 1
                    addDisplayCharacterAtPosition(handler, handler->position, createCharacter(character));
                    break;
                }
            }

            renderInput(mainInput, handler);
        }

        renderCursor(mainInput, handler, mainInput->window.width/2 - handler->buffer[handler->position].bounds.x, -(elementToManipulate->bounds.y + elementToManipulate->bounds.height/2 - mainInput->window.y - mainInput->window.height/2), false);

        previousKey = key;
    } while(key != sk_Enter);
}

void renderInput(InputHandler* mainHandler, InputHandler* focusHandler) {
    DisplayCharacter* focusElement = &focusHandler->buffer[focusHandler->position];

    gfx_SetDrawBuffer();
    gfx_SetColor(inputBackgroundColor);
    gfx_FillRectangle_NoClip(mainHandler->window.x, mainHandler->window.y, mainHandler->window.width, mainHandler->window.height);

    setInputHandlerPosition(mainHandler, mainHandler->scale, mainHandler->window.height/2, mainHandler->window.x, mainHandler->window.y); 
    int offsetX = mainHandler->window.width/2 - focusElement->bounds.x;
    int offsetY = -(focusElement->bounds.y + focusElement->bounds.height/2 - mainHandler->window.y - mainHandler->window.height/2);

    // sets the character to middle of input, if there is a gap before the focus element shift the elements to left
    if(mainHandler->buffer[0].type != EMPTY_CHARACTER) {
        int furthestVisualLeftX = offsetX + mainHandler->buffer[0].bounds.x;
        if(furthestVisualLeftX > mainHandler->window.x) {
            offsetX -= furthestVisualLeftX - mainHandler->window.x; //take away the difference from the vidual x to the begining of input to align the input to begining
        }
    }

    renderInputHandler(mainHandler, offsetX, offsetY);

    gfx_BlitRectangle(gfx_buffer, mainHandler->window.x, mainHandler->window.y, mainHandler->window.width, mainHandler->window.height);
    gfx_SetDrawScreen();
}

void insertDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character) {
    if(!handler || !handler->buffer || !character || position < 0 || position >= handler->size) return;

    Input_shiftElementsRight(handler, position);
    handler->buffer[position] = *character;
}

void addDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character) { // handles empty character stuff, use as default
    if(!handler || !handler->buffer || !character || position < 0 || position >= handler->size) return;

    if(handler->buffer[handler->position].type == GAP_CHARACTER) {
        insertDisplayCharacterAtPosition(handler, handler->position, character);
        handler->position++;
    }
    else if(handler->size - 1 == position) {
        appendDisplayCharacter(handler, character);
        handler->position++;
    } 
    else {
        setDisplayCharacterAtPosition(handler, position, character);
        handler->position++;
    }
}

void appendDisplayCharacter(InputHandler* handler, DisplayCharacter* character) {
    if(!handler || !handler->buffer || !character) return;

    // if(handler->size >= handler->capacity)

    handler->buffer[handler->size - 1] = *character;
    handler->buffer[handler->size++] = *createEmptyCharacter(); // add a new empty character ao you can move cursor to position

}

void setDisplayCharacterAtPosition(InputHandler* handler, int position, DisplayCharacter* character) {
    if(!handler || !handler->buffer || !character || position < 0 || position >= handler->size) return;

    handler->buffer[position] = *character;
}

void deleteCharacterAtPosition(InputHandler* handler, int position, bool force) {
    if(!handler || !handler->buffer || position < 0 || position >= handler->size) return;
    if(!force && (handler->buffer[handler->position].type == PLACEHOLDER_CHARACTER || handler->buffer[handler->position].type == EMPTY_CHARACTER || handler->buffer[handler->position].type == GAP_CHARACTER)) return;


    if(handler->buffer[position].type == FRACTION_CHARACTER || handler->buffer[position].type == EXPONENT_CHARACTER || handler->buffer[position].type == FUNCTION_CHARACTER) {
        // remove gap character as well
        Input_shiftElementsLeft(handler, --position);
    }

    Input_shiftElementsLeft(handler, position);

    if(handler->position >= handler->size) {
        handler->position = handler->size - 1;
    }

    if(handler->size == 0 || (handler->size == 1 && handler->buffer[0].type == EMPTY_CHARACTER)) {
        handler->buffer[0] = *createPlaceHolderCharacter();
        handler->size = 1;
        handler->position = 0;
    }
}

int Input_moveLeft(InputHandler* handler) {
    if(!handler || !handler->buffer) return -1;

    if(handler->position > 0) {
        handler->position--;

        if(handler->buffer[handler->position].type == FRACTION_CHARACTER) {
            handler->inFocus = false;
            recordInput(handler->buffer[handler->position].data.fraction->denominator);
            return 1; // return 1 to exit recordInput loop
        }
        if(handler->buffer[handler->position].type == EXPONENT_CHARACTER) {
            handler->inFocus = false;
            recordInput(handler->buffer[handler->position].data.exponent->exponent);
            return 1; // return 1 to exit recordInput loop
        }
        if(handler->buffer[handler->position].type == FUNCTION_CHARACTER) {
            handler->inFocus = false;
            recordInput(handler->buffer[handler->position].data.function->input);
            return 1; // return 1 to exit recordInput loop
        }
    } else if(handler->leftHandler) {
        // move to next left handler if it exists
        handler->inFocus = false;

        InputHandler* leftHandler = handler->leftHandler;
        while(leftHandler->buffer[leftHandler->position].type == FRACTION_CHARACTER || leftHandler->buffer[leftHandler->position].type == EXPONENT_CHARACTER) {
            int result = Input_moveLeft(leftHandler);
            if(result == 1) return 1; // return 1 to exit recordInput loop
        }
        recordInput(handler->leftHandler);
        return 1; // return 1 to exit recordInput loop
    }

    return 0; // return 0 for normal operation
}

int Input_moveRight(InputHandler* handler) {
    if(!handler || !handler->buffer) return -1;

    if(handler->position < handler->size - 1) {
        if(handler->buffer[handler->position].type == FUNCTION_CHARACTER) {
            handler->inFocus = false;

            //if(Function_getName(&handler->buffer[handler->position]) != "#") handler->position++; // if function has a name go straight into inout with position shift

            if(handler->buffer[handler->position].data.function->baseInput) {
                recordInput(handler->buffer[handler->position].data.function->baseInput);
                return 1; // return 1 to exit recordInput loop
            }

            recordInput(handler->buffer[handler->position].data.function->input);
            return 1; // return 1 to exit recordInput loop
        }

        handler->position++;

        if(handler->buffer[handler->position].type == FRACTION_CHARACTER) {
            handler->inFocus = false;
            recordInput(handler->buffer[handler->position].data.fraction->numerator);
            return 1; // return 1 to exit recordInput loop
        }
        if(handler->buffer[handler->position].type == EXPONENT_CHARACTER) {
            handler->inFocus = false;
            recordInput(handler->buffer[handler->position].data.exponent->exponent);
            return 1; // return 1 to exit recordInput loop
        }
    } else if(handler->rightHandler) {
        // move to next right handler if it exists
        handler->inFocus = false;

        InputHandler* rightHandler = handler->rightHandler;

        if(rightHandler->buffer[rightHandler->position].type == FUNCTION_CHARACTER) {
            rightHandler->position++;
        }

        while(rightHandler->buffer[rightHandler->position].type == FRACTION_CHARACTER || rightHandler->buffer[rightHandler->position].type == EXPONENT_CHARACTER) {
            int result = Input_moveRight(rightHandler);
            if(result == 1) return 1; // return 1 to exit recordInput loop
        }
        recordInput(handler->rightHandler);
        return 1; // return 1 to exit recordInput loop
    }

    return 0; // return 0 for normal operation
}

void Input_shiftElementsLeft(InputHandler* handler, int position) {
    if(!handler || !handler->buffer) return;

    // shifts all elements to the left
    // starts at positon and move all elements to the right of that position

    DisplayCharacter* focusElement = &handler->buffer[position];
    for(int i = position; i < handler->size - 1; i++) {
        handler->buffer[i] = handler->buffer[i + 1];
    }

    handler->size--;
}

void Input_shiftElementsRight(InputHandler* handler, int position) {
    if(!handler || !handler->buffer) return;
    if(handler->size - 1 >= handler->capacity) return;

    // shifts all elements to the right
    // all elements to the right of position are moved to the right

    bool lastElementExists = handler->size >= handler->capacity;

    for(int i = handler->size + 1; i > position; i--) {
        handler->buffer[i] = handler->buffer[i - 1];
    }

    if(!lastElementExists) handler->size++;

}   

void renderCursor(InputHandler* mainHandler, InputHandler* handler, int offsetX, int offsetY, bool nullCursor) {
    static uint_fast8_t ticker = 0;
    static uint8_t tickSpeed = 1;

    int position = handler->position;
    DisplayCharacter* focusElement = &handler->buffer[position];
    int CursorX, CursorY, CursorWidth, CursorHeight;

    CursorWidth = focusElement->bounds.width;
    CursorHeight = focusElement->bounds.height;

    if(!CursorHeight || !CursorWidth) {
        CursorWidth = getDisplayCharacterWidth(&handler->buffer[position]);
        CursorHeight = getDisplayCharacterHeight(&handler->buffer[position], handler->scale);

        CursorX = focusElement->bounds.x;
        CursorY = focusElement->bounds.y;
    } else {
        CursorX = focusElement->bounds.x;
        CursorY = focusElement->bounds.y;
    }
    

    if(mainHandler->buffer[0].type != EMPTY_CHARACTER) { // mainHandler->buffer[0].type != GAP_CHARACTER 
        int furthestVisualLeftX = offsetX + mainHandler->buffer[0].bounds.x;
        if(furthestVisualLeftX > mainHandler->window.x) {
            offsetX -= furthestVisualLeftX - mainHandler->window.x; //take away the difference from the vidual x to the begining of input to align the input to begining
        }
    }

    CursorX += offsetX;
    CursorY += offsetY;

    gfx_SetDrawBuffer();

    if(ticker > 128) {
        //no render cursor
        // the 2 is just how much it iterates per tick
        if(ticker - tickSpeed <= 128) {
            //render the node, previously erased
            gfx_SetColor(inputBackgroundColor);
            gfx_FillRectangle(CursorX, CursorY, CursorWidth, CursorHeight);
            if(focusElement->type != EMPTY_CHARACTER && focusElement->type != GAP_CHARACTER) renderDisplayCharacter(focusElement, handler->scale, offsetX, offsetY);

            gfx_BlitRectangle(gfx_buffer, CursorX, CursorY, CursorWidth, CursorHeight);
            gfx_SetDrawScreen();
        }

        ticker += tickSpeed;
        return;
    }

    gfx_SetColor(inputTextColor);
    gfx_FillRectangle(CursorX, CursorY, CursorWidth, CursorHeight);

    // special cases for cursor
    if(nullCursor) {
        gfx_SetColor(100); // temp
    }

    gfx_SetTextFGColor(inputBackgroundColor);
    if(focusElement->type == GAP_CHARACTER) {
        renderCharAt(Character_RightArrow, CursorX, CursorY, handler->scale);
    }
    else if(secondEnabled && alphaEnabled /*&& CursorWidth >= 6*/) { // underlined 'A'
        renderCharAt(Character_SecondAlphaCursorIdentifier, CursorX, CursorY, handler->scale);
    }
    else if(secondEnabled /*&& CursorWidth >= 6*/) { //up arrow
        renderCharAt(Character_SecondCursorIdentifier, CursorX, CursorY, handler->scale);
    }
    else if(alphaEnabled /*&& CursorWidth >= 6*/) { //an 'A'
        renderCharAt(Character_AlphaCursorIdentifier, CursorX, CursorY, handler->scale);
    }

    gfx_BlitRectangle(gfx_buffer, CursorX, CursorY, CursorWidth, CursorHeight);
    gfx_SetDrawScreen();

    ticker += tickSpeed;
}

bool manageSpecialKeyStates(uint8_t key) {
    if(!secondEnabled) {
        alphaEnabled = false; // if a key was presed and alpha lock not on turn off alpha
    } else { // to fix
        secondEnabled = false; 
        alphaEnabled = false; 
    }

    if(key == sk_Alpha) {
        alphaEnabled = !alphaEnabled;
        return true;
    }
    else if(key == sk_2nd) {
        secondEnabled = !secondEnabled;
        return true;
    }

    return false;
}

FunctionName characterToFunctionCharacter (char character) {
    switch(character) {
        case Character_Sin: return SIN_FUNCTION;
        case Character_Cos: return COS_FUNCTION;
        case Character_Tan: return TAN_FUNCTION;
        case Character_Csc: return CSC_FUNCTION;
        case Character_Sec: return SEC_FUNCTION;
        case Character_Cot: return COT_FUNCTION;
        case Character_InvSin: return INVSIN_FUNCTION;
        case Character_InvCos: return INVCOS_FUNCTION;
        case Character_InvTan: return INVTAN_FUNCTION;
        case Character_InvCsc: return INVCSC_FUNCTION;
        case Character_InvSec: return INVSEC_FUNCTION;
        case Character_InvCot: return INVCOT_FUNCTION;
        case Character_Log: return LOG_FUNCTION;
        case Character_LogBase: return LOG_BASE_FUNCTION;
        case Character_Ln: return LN_FUNCTION;
        case Character_SquareRoot: return SQRT_FUNCTION;
        case Character_Root: return ROOT_FUNCTION;
        case Character_Absolute: return ABSOLUTE_FUNCTION;

        default: return SIN_FUNCTION;
    }
}