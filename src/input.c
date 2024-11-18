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

#define MAX_FUNCTION_INPUT_SIZE 128
#define INITIAL_CAPACITY 16
#define CAPACITY_INCREASE_FACTOR 24

//todo
/* 
    - fix jankiness with left and right movement w/ functions
    - if placeholder char is deleted then delete the corresponding special char (fraction, exp, func)

    - if you change top-left...etc parenthesis/brackets chars make sure you update -1 spacing in inv funcs
*/

void initializeFonts(void){
    gfx_SetFontData(TextData);
}

struct {
    int x;
    int y;
    int width;
    int height;
    uint_fast8_t ticker;
    Display_char* node;
} Cursor;

void setupFractionLinks(Input_handler* handler, Display_char* element) {
    element->data.fraction->numerator->left = handler;
    element->data.fraction->numerator->right = element->data.fraction->denominator;
    element->data.fraction->denominator->left = element->data.fraction->numerator;
    element->data.fraction->denominator->right = handler;
}
void setupExponentLinks(Input_handler* handler, Display_char* element) {
    element->data.exponent->exponent->left = handler;
    element->data.exponent->exponent->right = handler;
}
void setupFunctiontLinks(Input_handler* handler, Display_char* element) {
    element->data.function->input->left = handler;
    element->data.function->input->right = handler;

    if(element->data.function->baseInput) {
        element->data.function->input->left = element->data.function->baseInput;
        // right of main input remains the same
        element->data.function->baseInput->left = handler;
        element->data.function->baseInput->right = element->data.function->input;
    }
}
void setupFractionRendering(Display_char* element) {
    element->display = Fraction_render;
    element->setPosition = Fraction_setPosition;
    element->getAboveOriginHeight = Fraction_getAboveOriginHeight;
    element->getBelowOriginHeight = Fraction_getBelowOriginHeight;
    element->getWidth = Fraction_getWidth;
    element->getHeight = Fraction_getHeight;
}
void setupExponentRendering(Display_char* element) {
    element->display = Exponent_render;
    element->setPosition = Exponent_setPosition;
    element->getAboveOriginHeight = Exponent_getAboveOriginHeight;
    element->getBelowOriginHeight = Exponent_getBelowOriginHeight;
    element->getWidth = Exponent_getWidth;
    element->getHeight = Exponent_getHeight;
}
void setupFunctionRendering(Display_char* element) {
    element->display = Function_render;
    element->setPosition = Function_setPosition;
    element->getAboveOriginHeight = Function_getAboveOriginHeight;
    element->getBelowOriginHeight = Function_getBelowOriginHeight;
    element->getWidth = Function_getWidth;
    element->getHeight = Function_getHeight;
}
bool isValidExponentPosition(Input_handler* handler) {
    if (handler->position == 0) return false;
    CharacterType prevType = handler->buffer[handler->position - 1].type;
    return !(prevType == EMPTY_CHARACTER || prevType == FRACTION_CHARACTER || prevType == EXPONENT_CHARACTER);
}
void initializeFractionPlaceholders(Display_char* element, int scale) {
    createPlaceholderChar(&element->data.fraction->numerator->buffer[0], scale);
    element->data.fraction->numerator->size = 1;
    
    createPlaceholderChar(&element->data.fraction->denominator->buffer[0], scale);
    element->data.fraction->denominator->size = 1;
}
void initializeExponentPlaceholder(Display_char* element, int scale) {
    createPlaceholderChar(&element->data.exponent->exponent->buffer[0], scale);
    element->data.exponent->exponent->size = 1;
}
void initializeFunctionPlaceholders(Display_char* element, int scale) {
    createPlaceholderChar(&element->data.function->input->buffer[0], scale);
    element->data.function->input->size = 1;

    if(!element->data.function->baseInput) return;
    //create placeholder for baseInput
    createPlaceholderChar(&element->data.function->baseInput->buffer[0], scale);
    element->data.function->baseInput->size = 1;
}
void initializeDisplayChar(Display_char* element, int scale) {
    element->scale = scale;
    element->display = Character_render;
    element->setPosition = Character_setPosition;
    element->getAboveOriginHeight = Character_getAboveOriginHeight;
    element->getBelowOriginHeight = Character_getBelowOriginHeight;
    element->getWidth = Character_getWidth;
    element->getHeight = Character_getHeight;
}
void createPlaceholderChar(Display_char* element, int scale) {
    element->type = PLACEHOLDER_CHARACTER;
    element->data.variable = Character_PlaceHolder;
    element->scale = scale;
    initializeDisplayChar(element, scale);
}
void createGapChar(Display_char* element, int scale) {
    element->type = GAP_CHARACTER;
    element->scale = scale;
    initializeDisplayChar(element, scale);
}
Input_handler* createFraction(Input_handler* handler, Display_char* elementToManipulate) {
    // Handle existing element
    if (elementToManipulate->type != EMPTY_CHARACTER) {
        if (elementToManipulate->type == GAP_CHARACTER) {
            Input_shiftRight(handler, handler->position);
            handler->size--;
            Input_shiftRight(handler, handler->position);
            handler->size--;

            elementToManipulate = &handler->buffer[handler->position];
        } else {
            freeCharacter(elementToManipulate);
            handler->size--;
        }
    }

    // Create gap character
    createGapChar(elementToManipulate, handler->scale);
    handler->position++;
    handler->size++;

    // Setup fraction character
    elementToManipulate = &handler->buffer[handler->position];
    elementToManipulate->type = FRACTION_CHARACTER;
    elementToManipulate->data.fraction = malloc(sizeof(Fraction_char));

    if(elementToManipulate->data.fraction == NULL) return NULL;

    elementToManipulate->scale = (int)fmax(handler->scale / 2, 1);

    // Initialize numerator and denominator
    elementToManipulate->data.fraction->numerator = NULL;
    elementToManipulate->data.fraction->denominator = NULL;

    elementToManipulate->data.fraction->numerator = createInputHandler(handler->maxSize);
    if(elementToManipulate->data.fraction->numerator == NULL) return NULL;
    elementToManipulate->data.fraction->denominator = createInputHandler(handler->maxSize);
    if(elementToManipulate->data.fraction->denominator == NULL) return NULL;

    // Set up navigation links
    setupFractionLinks(handler, elementToManipulate);

    // Set up fraction-specific rendering functions
    setupFractionRendering(elementToManipulate);

    // Initialize placeholder characters
    initializeFractionPlaceholders(elementToManipulate, elementToManipulate->scale);

    handler->size++;
    return elementToManipulate->data.fraction->numerator;
}
Input_handler* createExponent(Input_handler* handler, Display_char* elementToManipulate) {
    // Validate position for exponent
    if (!isValidExponentPosition(handler)) return NULL; //invalid position is when previous char is raw fraction, another exponent, etc

    // Handle existing element
    if (elementToManipulate->type != EMPTY_CHARACTER) {
        if (elementToManipulate->type == GAP_CHARACTER) {
            Input_shiftRight(handler, handler->position);
            handler->size--;
            Input_shiftRight(handler, handler->position);
            handler->size--;

            elementToManipulate = &handler->buffer[handler->position];
        } else {
            freeCharacter(elementToManipulate);
            handler->size--;
        }
    }

    // Create gap character
    createGapChar(elementToManipulate, handler->scale);
    handler->position++;
    handler->size++;

    // Setup exponent character
    elementToManipulate = &handler->buffer[handler->position];
    elementToManipulate->type = EXPONENT_CHARACTER;
    elementToManipulate->data.exponent = malloc(sizeof(Exponent_char));

    if (elementToManipulate->data.exponent == NULL) return NULL;

    elementToManipulate->scale = (int)fmax(handler->scale / 2, 1);

    // Initialize exponent input handler
    elementToManipulate->data.exponent->exponent = createInputHandler(handler->maxSize);

    if(elementToManipulate->data.exponent->exponent == NULL) return NULL;
    
    // Set up navigation links
    setupExponentLinks(handler, elementToManipulate);

    // Set up exponent-specific rendering functions
    setupExponentRendering(elementToManipulate);

    // Initialize placeholder character
    initializeExponentPlaceholder(elementToManipulate, elementToManipulate->scale);

    handler->size++;
    return elementToManipulate->data.exponent->exponent;
}
Input_handler* createFunction(Input_handler* handler, Display_char* elementToManipulate, FunctionName name, int base, bool hasBorder) {
    if(!elementToManipulate) return NULL;

    // Handle existing element
    if (elementToManipulate->type != EMPTY_CHARACTER) {
        if (elementToManipulate->type == GAP_CHARACTER) {
            //Input_shiftRight(handler, handler->position);
            //handler->size--;
            //elementToManipulate = &handler->buffer[handler->position];
        } else {
            freeCharacter(elementToManipulate);
            handler->size--;
        }
    }

    elementToManipulate->data.function = malloc(sizeof(Function_char));
    if (elementToManipulate->data.function == NULL) return NULL;

    elementToManipulate->scale = handler->scale;
    if(hasBorder) elementToManipulate->data.function->border = Charcater_Top_Open_Parenthesis;
    elementToManipulate->data.function->name = name;

    // Initialize exponent input handler
    elementToManipulate->data.function->input = createInputHandler(MAX_FUNCTION_INPUT_SIZE);
    if (elementToManipulate->data.function->input == NULL) return NULL;

    elementToManipulate->data.function->baseInput = NULL;

    if(name == LOG_BASE_FUNCTION || name == ROOT_FUNCTION) {
        //require base inputs
        elementToManipulate->data.function->baseInput = createInputHandler(8); //shoudnt be larger than 8 chars 2 even 
        if (elementToManipulate->data.function->baseInput == NULL) {
            freeInputHandler(elementToManipulate->data.function->input);
            free(elementToManipulate->data.function);
            elementToManipulate->data.function = NULL;
            elementToManipulate->type = EMPTY_CHARACTER;
            return NULL;
        }
    }
    
    // Set up navigation links
    setupFunctiontLinks(handler, elementToManipulate);

    // Set up exponent-specific rendering functions
    setupFunctionRendering(elementToManipulate);

    // Initialize placeholder character
    initializeFunctionPlaceholders(elementToManipulate, elementToManipulate->scale);

    handler->position++;
    elementToManipulate->type = FUNCTION_CHARACTER;
    handler->size++;
    return elementToManipulate->data.function->input;

}

void renderStringAt(const char* string, int x, int y, int scale) {
    int width = 0;
    while (*string) {
        renderCharAt(*string,x + width,y,scale);
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

void Character_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY) {
    int x = currentBounds->x + currentBounds->width;
    int y = originY - node->getBelowOriginHeight(node);
    node->x = x + offsetX;
    node->y = y + offsetY;

    if(node->type != GAP_CHARACTER || Cursor.node == node) currentBounds->width += node->getWidth(node); 
}
void Character_render(Display_char* node, int offsetX, int offsetY) {
    

    if(node->type == GAP_CHARACTER) {
        //char string[2] = {Character_RightArrow, '\0'};
        gfx_SetTextFGColor(inputBackgroundColor);
        renderCharAt(Character_RightArrow, node->x + offsetX, node->y + offsetY, node->scale);
        return;
    }

    if(node->type == CURSOR_CHARCATER) return;

    //char string[2] = {node->data.variable, '\0'};

    gfx_SetTextFGColor(inputTextColor);
    renderCharAt(node->data.variable, node->x + offsetX, node->y + offsetY, node->scale);
    //gfx_SetTextScale(1, node->scale); 
    //gfx_PrintStringXY(string, node->x + offsetX, node->y + offsetY);
}
int Character_getAboveOriginHeight(Display_char* node) {
    return node->getHeight(node) / 2;
}
int Character_getBelowOriginHeight(Display_char* node) {
    return node->getHeight(node) / 2;
}
int Character_getWidth(Display_char* node) {
    if(node->type == GAP_CHARACTER && node != Cursor.node) return 0;
    return fmax(gfx_GetCharWidth(node->data.variable), 4); //minimum 4
}
int Character_getHeight(Display_char* node) {
    if(node->type == GAP_CHARACTER && node != Cursor.node) return 0;
    return 8 * node->scale; //a constant
}
void Fraction_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY) {
    int x = currentBounds->x + currentBounds->width;
    int y = originY;

    int numHeight = Fraction_getAboveOriginHeight(node);
    //int denHeight = Fraction_getBelowOriginHeight(*node);
    int numWidth = getExpressionWidth(node->data.fraction->numerator);
    int denWidth = getExpressionWidth(node->data.fraction->denominator);
    int maxWidth = fmax(fmax(numWidth, denWidth), 4); //minimum 4

    node->data.fraction->numerator->currentBounds->y = originY - numHeight;
    node->data.fraction->denominator->currentBounds->y = originY + 2;
    node->data.fraction->numerator->currentBounds->x = numWidth == maxWidth ? x : x + (maxWidth - numWidth) / 2;
    node->data.fraction->denominator->currentBounds->x = denWidth == maxWidth ? x : x + (maxWidth - denWidth) / 2;

    setInputCharacterPositions(node->data.fraction->numerator, offsetX, 0);
    setInputCharacterPositions(node->data.fraction->denominator, offsetX, 0);

    node->x = x + offsetX;
    node->y = y - numHeight + offsetY; 

    currentBounds->width += maxWidth;
}
void Fraction_render(Display_char* node, int offsetX, int offsetY) {
    gfx_SetColor(inputTextColor);
    gfx_HorizLine(node->x + offsetX, node->y + offsetY + node->getAboveOriginHeight(node), fmin(node->getWidth(node), GFX_LCD_WIDTH - (node->x + offsetX))); //clamps line to screen

    renderInput(node->data.fraction->numerator, false, offsetX, offsetY);
    renderInput(node->data.fraction->denominator, false, offsetX, offsetY);
}
int Fraction_getAboveOriginHeight(Display_char* node) {
    return fmax(getExpressionHeight(node->data.fraction->numerator),8 * node->scale) + 1; //minimum 8 
}
int Fraction_getBelowOriginHeight(Display_char* node) {
    return fmax(getExpressionHeight(node->data.fraction->denominator), 8 * node->scale) + 2;
}
int Fraction_getWidth(Display_char* node) {
    return fmax(fmax(getExpressionWidth(node->data.fraction->numerator), getExpressionWidth(node->data.fraction->denominator)), 8); //minimum 8
}
int Fraction_getHeight(Display_char* node) {
    return Fraction_getAboveOriginHeight(node) + Fraction_getBelowOriginHeight(node) + 2; //+2 to account for bar
}

void Exponent_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY) {
    int height = Exponent_getHeight(node);

    int x = currentBounds->x + currentBounds->width;
    int y = originY - height + 4 * node->scale;

    node->data.exponent->exponent->currentBounds->y = y;
    node->data.exponent->exponent->currentBounds->x = x;

    setInputCharacterPositions(node->data.exponent->exponent, offsetX, 0);

    node->x = x + offsetX;
    node->y = y + offsetY;

    currentBounds->width += Exponent_getWidth(node);
}
void Exponent_render(Display_char* node, int offsetX, int offsetY) {
    renderInput(node->data.exponent->exponent, false, offsetX, offsetY);
}
int Exponent_getAboveOriginHeight(Display_char* node) {
    return fmax(getExpressionHeight(node->data.exponent->exponent),8 * node->scale); //minimum 8 
}
int Exponent_getBelowOriginHeight(Display_char* node) {
    return fmax(getExpressionHeight(node->data.exponent->exponent), 8 * node->scale) - 4 * node->scale;
}
int Exponent_getWidth(Display_char* node) {
    return getExpressionWidth(node->data.exponent->exponent);
}
int Exponent_getHeight(Display_char* node) {
    return fmax(getExpressionHeight(node->data.exponent->exponent), 8 * node->scale) + 4 * node->scale; //the plus 4 is to account for exponent shift 
}

bool functionHasNegativeOnePower(Display_char* node) {
    //if functions like sin^-1(x) have the negative one
    return !preferARCsyntax && (node->data.function->name == INVSIN_FUNCTION || node->data.function->name == INVCOS_FUNCTION || node->data.function->name == INVTAN_FUNCTION || node->data.function->name == INVCSC_FUNCTION || node->data.function->name == INVSEC_FUNCTION || node->data.function->name == INVCOT_FUNCTION);
}
void Function_setPosition(Display_char* node, Bounds_char* currentBounds, int originY, int offsetX, int offsetY) {
    int aboveHeight = Function_getAboveOriginHeight(node);
    int x = currentBounds->x + currentBounds->width;
    int y = originY - aboveHeight;

    if (node->data.function->input != NULL) {
        node->data.function->input->currentBounds->y = y + (functionHasNegativeOnePower(node) ? 4 * node->scale : 0) + (node->data.function->name == SQRT_FUNCTION || node->data.function->name == ROOT_FUNCTION ? 2 : 0 ); //account for -2
        node->data.function->input->currentBounds->x = x;
        int openBorderWidth = node->data.function->border ? gfx_GetCharWidth(node->data.function->border) : 0;
        setInputCharacterPositions(node->data.function->input, Function_getNameWidth(node) + openBorderWidth + offsetX, 0);
    }

    if(node->data.function->baseInput != NULL) {
        if(node->data.function->name == ROOT_FUNCTION) {
            node->data.function->baseInput->currentBounds->y = y; //- getExpressionHeight(node->data.function->baseInput) + getExpressionHeight(node->data.function->input);
            node->data.function->baseInput->currentBounds->x = x + offsetX;
        } else if(node->data.function->name == LOG_BASE_FUNCTION) {
            node->data.function->baseInput->currentBounds->y = y + node->getHeight(node) - 4 * node->scale;
            node->data.function->baseInput->currentBounds->x = x + (3 * 8) + offsetX; //3 mono chars
        }

        setInputCharacterPositions(node->data.function->baseInput, 0, 0);
    }

    node->x = x + offsetX;
    node->y = y + offsetY;

    currentBounds->width += Function_getWidth(node);
}
void Function_render(Display_char* node, int offsetX, int offsetY) {
    typedef struct {
        const char* name;
        size_t length;
    } FunctionDisplay;
    FunctionDisplay textToRender = {"",0};
    bool specialRendering = false;

    switch(node->data.function->name) {
        //all functions with text
        case SIN_FUNCTION: textToRender = (FunctionDisplay){"Sin", 3}; break;
        case COS_FUNCTION: textToRender = (FunctionDisplay){"Cos", 3}; break;
        case TAN_FUNCTION: textToRender = (FunctionDisplay){"Tan", 3}; break;
        case CSC_FUNCTION: textToRender = (FunctionDisplay){"Csc", 3}; break;
        case SEC_FUNCTION: textToRender = (FunctionDisplay){"Sec", 3}; break;
        case COT_FUNCTION: textToRender = (FunctionDisplay){"Cot", 3}; break;
        case INVSIN_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcSin", 6} : (FunctionDisplay){"Sin", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case INVCOS_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcCos", 6} : (FunctionDisplay){"Cos", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case INVTAN_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcTan", 6} : (FunctionDisplay){"Tan", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case INVCSC_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcCsc", 6} : (FunctionDisplay){"Csc", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case INVSEC_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcSec", 6} : (FunctionDisplay){"Sec", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case INVCOT_FUNCTION: {
            textToRender = preferARCsyntax ? (FunctionDisplay){"ArcCot", 6} : (FunctionDisplay){"Cot", 3}; 
            if(!preferARCsyntax) specialRendering = true;
            break;
        }
        case LOG_FUNCTION: 
        case LOG_BASE_FUNCTION: 
            textToRender = (FunctionDisplay){"Log", 3}; break;
        case LN_FUNCTION: textToRender = (FunctionDisplay){"Ln", 2}; break;
        case ABSOLUTE_FUNCTION: 
        case SQRT_FUNCTION:
        case ROOT_FUNCTION:
            break; //just here to suppress warnings
    }

    if(textToRender.length > 0) {
        //has text to render
        int aboveHeight = node->getAboveOriginHeight(node);
        gfx_SetTextFGColor(inputTextColor);
        renderStringAt(textToRender.name, node->x + offsetX, node->y + offsetY - 4 * node->scale + aboveHeight, node->scale);//- 4 is inintial height height of char / 2; 
        if(node->data.function->border) {
            int belowHeight = node->getBelowOriginHeight(node);
            int topOffsetY = -(aboveHeight - 4 * node->scale) - 4 * node->scale + aboveHeight;
            int bottomOffsetY = (belowHeight - 4 * node->scale) - 4 * node->scale + aboveHeight;

            // addaptive parenthesis height rendering
            renderCharAt(node->data.function->border, node->x + offsetX + Function_getNameWidth(node), node->y + offsetY + topOffsetY, node->scale); 
            renderCharAt(node->data.function->border + 1, node->x + offsetX + Function_getNameWidth(node), node->y + offsetY + bottomOffsetY, node->scale); 
            renderCharAt(node->data.function->border + 2, node->x + offsetX + Function_getWidth(node) - (node->data.function->border ? gfx_GetCharWidth(node->data.function->border) : 0), node->y + offsetY + topOffsetY, node->scale);
            renderCharAt(node->data.function->border + 3, node->x + offsetX + Function_getWidth(node) - (node->data.function->border ? gfx_GetCharWidth(node->data.function->border + 2) : 0), node->y + offsetY + bottomOffsetY, node->scale); 

            int length = bottomOffsetY - topOffsetY;
            gfx_SetColor(inputTextColor);
            gfx_VertLine(node->x + offsetX + Function_getNameWidth(node) + 3,node->y + offsetY + topOffsetY + 4 * node->scale,length); //left 
            gfx_VertLine(node->x + offsetX + Function_getWidth(node) - (node->data.function->border ? gfx_GetCharWidth(node->data.function->border) : 0) + 4,node->y + offsetY + topOffsetY + 4,length); //right 
        }
        if(specialRendering) {
            if(functionHasNegativeOnePower(node)) {
                //rendering of the -1 (8 * 3 * node->scale - 4 * node->scale); - 4 for shifted -1 to take up less space and move up vertically
                renderStringAt("-1", node->x + offsetX + (18), node->y + offsetY - 8 * node->scale + aboveHeight, fmax((int)node->scale/2,1));
            }
        }
    }
    else {
        //no text / special cases
        if(node->data.function->name == SQRT_FUNCTION || node->data.function->name == ROOT_FUNCTION) {
            // square root function
            int nameWidth = Function_getNameWidth(node);
            int height = node->getHeight(node);
            int inputWidth = getExpressionWidth(node->data.function->input);

            gfx_SetColor(inputTextColor);
            gfx_Line(node->x + offsetX + nameWidth - 6, node->y + offsetY + height - 2, node->x + offsetX + nameWidth - 2, node->y + offsetY + height);
            gfx_VertLine(node->x + offsetX + nameWidth - 2, node->y + offsetY, height); // side bar 
            gfx_HorizLine(node->x + offsetX + nameWidth - 2, node->y + offsetY, inputWidth + 2); // top bar 
        }
        else if(node->data.function->name == ABSOLUTE_FUNCTION) {
            int length = getExpressionHeight(node->data.function->input);
            gfx_SetColor(inputTextColor);
            gfx_VertLine(node->x + offsetX + 3,node->y + offsetY, length); //left 
            gfx_VertLine(node->x + offsetX + Function_getWidth(node) - 4,node->y + offsetY,length); //right 
        }
    }

    if (node->data.function->input != NULL) {
        renderInput(node->data.function->input, false, offsetX, offsetY);
    }
    if (node->data.function->baseInput != NULL) {
        renderInput(node->data.function->baseInput, false, offsetX, offsetY);
    }
}
int Function_getAboveOriginHeight(Display_char* node) {
    int additionalPadding = 0;

    if(functionHasNegativeOnePower(node)) additionalPadding += 4 * node->scale; //account for -1
    else if(node->data.function->name == SQRT_FUNCTION ) additionalPadding += 2; // account for top bar of root
    else if(node->data.function->name == ROOT_FUNCTION) additionalPadding += 3 + getExpressionHeight(node->data.function->baseInput) - node->scale * 8; //account for -2

    return fmax(getExpressionAboveOriginHeight(node->data.function->input), 4 * node->scale) + additionalPadding; //minimum 4
}
int Function_getBelowOriginHeight(Display_char* node) {
    return fmax(getExpressionBelowOriginHeight(node->data.function->input), 4 * node->scale);
}
int Function_getWidth(Display_char* node) {
    int functionNameWidth = Function_getNameWidth(node);
    int openBorderWidth = node->data.function->border ? gfx_GetCharWidth(node->data.function->border) : 0;
    int closedBorderWidth = node->data.function->border ? gfx_GetCharWidth(node->data.function->border + 2) : 0;

    return functionNameWidth + openBorderWidth + getExpressionWidth(node->data.function->input) + closedBorderWidth;
}
int Function_getHeight(Display_char* node) {
    int additionalPadding = 0;

    if(functionHasNegativeOnePower(node)) additionalPadding += 4 * node->scale; //account for -1
    else if(node->data.function->name == SQRT_FUNCTION ) additionalPadding += 2; // account for top bar of root
    else if(node->data.function->name == ROOT_FUNCTION) additionalPadding += 3 + getExpressionHeight(node->data.function->baseInput) - node->scale * 8; //account for -2

    return fmax(getExpressionHeight(node->data.function->input), 8 * node->scale) + additionalPadding; 
}
int Function_getNameWidth(Display_char* node) {
    switch(node->data.function->name) {
        case INVSIN_FUNCTION: 
        case INVCOS_FUNCTION: 
        case INVTAN_FUNCTION: 
        case INVCSC_FUNCTION: 
        case INVSEC_FUNCTION: 
        case INVCOT_FUNCTION: 
            return preferARCsyntax ? 6 * 8 : 3.5 * 8 + 2; //ammount of chars * 8 (char width) 
        case SIN_FUNCTION: 
        case COS_FUNCTION: 
        case TAN_FUNCTION: 
        case CSC_FUNCTION: 
        case SEC_FUNCTION: 
        case COT_FUNCTION: 
        case LOG_FUNCTION: 
            return 8 * 3; //3 mono chars
        case ROOT_FUNCTION:
            return fmax(6, getExpressionWidth(node->data.function->baseInput)) + 4;
        case SQRT_FUNCTION:
            return 6; //root symbol
        case ABSOLUTE_FUNCTION:
            return 0;
        case LN_FUNCTION:
            return 8 * 2;
        case LOG_BASE_FUNCTION:
            return 8 * 3 + getExpressionWidth(node->data.function->baseInput); //3 chars + base width
    }
    return 8; //default char width
}



void Input_shiftLeft(Input_handler* handler, int initialPosition) {
    freeCharacter(&handler->buffer[initialPosition]); //remove starting char

    for(int index = initialPosition; index < handler->size; index++) {
        handler->buffer[index] = handler->buffer[index + 1];
    }

    freeCharacter(&handler->buffer[handler->maxSize - 1]); //remove very last char
    handler->size--;
}
void Input_shiftRight(Input_handler* handler, int initialPosition) {
    handler->size = fmin(handler->size + 1, handler->maxSize);
    freeCharacter(&handler->buffer[handler->maxSize - 1]); //remove very last char

    for (int index = handler->size; index > initialPosition; index--) {
        handler->buffer[index] = handler->buffer[index - 1];
    }

    freeCharacter(&handler->buffer[initialPosition]); //remove starting char
}
int Input_moveLeft(Input_handler* handler) {
    if (handler->position > 0) {
        handler->position--;

        if (handler->buffer[handler->position].type == FRACTION_CHARACTER) {
            deleteLastCursor(handler);
            recordInput(handler->buffer[handler->position].data.fraction->denominator, -1);
            return 1;  // Indicate that recordInput was called
        }
        if(handler->buffer[handler->position].type == EXPONENT_CHARACTER) {
            deleteLastCursor(handler);
            recordInput(handler->buffer[handler->position].data.exponent->exponent, -1);
            return 1;  // Indicate that recordInput was called
        }
        if(handler->buffer[handler->position].type == FUNCTION_CHARACTER) {
            deleteLastCursor(handler);
            recordInput(handler->buffer[handler->position].data.function->input, -1);
            return 1;  // Indicate that recordInput was called
        }
    } else if (handler->left) {
        deleteLastCursor(handler);
        recordInput(handler->left, -1);
        return 1;  // Indicate that recordInput was called
    }
    return 0;  // Indicate that recordInput was not called
}

int Input_moveRight(Input_handler* handler) {
    static bool justOnFunction;
    if(justOnFunction) {
        deleteLastCursor(handler);
        justOnFunction = false;
        //if(handler->buffer[handler->position].data.function->input->buffer[handler->buffer[handler->position].data.function->input->position].type != FUNCTION_CHARACTER) justOnFunction = false;
        recordInput(handler->buffer[handler->position].data.function->input, 1);
        return 1;
    }

    if (handler->position < handler->size && handler->buffer[handler->position].type != PLACEHOLDER_CHARACTER) {
        handler->position++;

        if(handler->buffer[handler->position].type == FUNCTION_CHARACTER) {
            justOnFunction = true;
            return 0;
        }


        if (handler->buffer[handler->position].type == FRACTION_CHARACTER) {
            deleteLastCursor(handler);
            recordInput(handler->buffer[handler->position].data.fraction->numerator, 1);
            return 1;  // Indicate that recordInput was called
        }
        if(handler->buffer[handler->position].type == EXPONENT_CHARACTER) {
            deleteLastCursor(handler);
            recordInput(handler->buffer[handler->position].data.exponent->exponent, 1);
            return 1;  // Indicate that recordInput was called
        }
    } else if (handler->right) {
        if(handler->right->buffer[handler->right->position].type != FUNCTION_CHARACTER) justOnFunction = false;
        deleteLastCursor(handler);
        recordInput(handler->right, 1);
        return 1;  // Indicate that recordInput was called
    }
    return 0;  // Indicate that recordInput was not called
}

const char *chars = "\0\0\0\0\0\0\0\0\0\0\"WRMH\0\0?[VQLG\0\0:ZUPKFC\0 YTOJEB\0\0XSNIDA\0\0\0\0\0\0\0\0";
//mode toggles
bool second = false; 
bool alpha = false;

uint8_t getKey(void) {
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
    if(alpha) {
        switch(key) {
            case sk_0: return Character_Fraction; //fraction - temp
            case sk_1: return Character_Root; // root - temp
            case sk_2: return Character_LogBase; // log base - temp
            case sk_3: return Character_Absolute; // absoluet func - temp
        }
        if(chars[key]) return chars[key];
    }
    if(second) {
        switch(key) {
            case sk_Power: return Character_PI; //PI
            case sk_Sin: return Character_InvSin; //sin^-1
            case sk_Cos: return Character_InvCos; //cos^-1
            case sk_Tan: return Character_InvTan; //tan^-1
            case sk_Square: return Character_SquareRoot;
            case sk_Log: return Character_TenPower; //10^
            case sk_Ln: return Character_Euler; //e
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


void recordInput(Input_handler* handler, int direction) {
    if (!handler || !handler->buffer) return;

    uint8_t previousKey = 0;
    uint8_t key = 0;

    //gets furthest left input 
    Input_handler* leftMostInputHandler = handler;
    while(leftMostInputHandler->left) {
        leftMostInputHandler = leftMostInputHandler->left;
    }

    Cursor.ticker = 0;
    deleteLastCursor(handler);

    if (handler->buffer[handler->position].type == FRACTION_CHARACTER || handler->buffer[handler->position].type == EXPONENT_CHARACTER || (handler->buffer[handler->position].type == FUNCTION_CHARACTER && direction == 1)) {
        int moved = (direction == 1) ? Input_moveRight(handler) : Input_moveLeft(handler);
        if (moved) return;  // Exit if recordInput was called within move function
    }


    //iniital render
    Cursor.node = &handler->buffer[handler->position];
    gfx_SetDrawBuffer();
    setCursorAtPosition(handler, handler->position);
    setInputCharacterPositions(leftMostInputHandler, 0, 0);
    positionCursor(handler, handler->position, -leftMostInputHandler->maxBounds->x, -leftMostInputHandler->maxBounds->y); //for position
    renderInput(leftMostInputHandler, true, leftMostInputHandler->maxBounds->width/2 - Cursor.x, leftMostInputHandler->maxBounds->height/2 - Cursor.y - Cursor.height/2);
    renderCursor(leftMostInputHandler, leftMostInputHandler->maxBounds->width/2 - Cursor.x, leftMostInputHandler->maxBounds->height/2 - Cursor.y - Cursor.height/2, false);
    // Swap to main buffer for display
    gfx_BlitRectangle(gfx_buffer, leftMostInputHandler->maxBounds->x, leftMostInputHandler->maxBounds->y, leftMostInputHandler->maxBounds->width, leftMostInputHandler->maxBounds->height);


    do {
        key = getKey();
        bool exceedsMaxSize = handler->size >= handler->maxSize;

        if(key && key != previousKey) {
            if(key == sk_Alpha) alpha = !alpha; //toggle
            if(key == sk_2nd) second = !second; //toggle

            char character = keyToChar(key);

            if(key != sk_Alpha && alpha && !second) {
                alpha = false; //reset alpha after key press
            }
            if(key != sk_2nd && second && !alpha) {
                second = false; //rest second after key press
            }
            if(handler->size >= handler->capacity - 3) {
                bool resized = resizeBuffer(handler, CAPACITY_INCREASE_FACTOR);
                if(!resized) continue; //skip this key press
            }
            
            Display_char* elementToManipulate = &handler->buffer[handler->position];
            deleteLastCursor(handler);

            // Handle key codes
            switch(character) {
                case Character_Delete:
                    if(handler->position > 0) {
                        // First properly free the character we're deleting
                        if(elementToManipulate->type == CURSOR_CHARCATER || elementToManipulate->type == GAP_CHARACTER) break; //cant delete cursor charcater or gap char

                        Input_shiftLeft(handler, handler->position);

                        //handler->position--;
                        //handler->size--;
                    }
                    break;

                case Character_Left:
                    Cursor.ticker = 0;
                    if (Input_moveLeft(handler)) return;  // Exit if recordInput was called within move function
                    break;

                case Character_Right:
                    Cursor.ticker = 0;
                    if (Input_moveRight(handler)) return;  // Exit if recordInput was called within move function
                    break;
                case Character_Up:
                    
                    break;

                case Character_Down:
                    
                    break;
                case Character_Fraction: {
                    if(exceedsMaxSize) break;
                    //creates fraction in place of elementToManipulate, returns numerator
                    Input_handler* numerator = createFraction(handler, elementToManipulate);
                    if(!numerator) break;
                    //records input for numeator initially
                    return recordInput(numerator, 1); 
                }
                case Character_Power: {  
                    if(exceedsMaxSize) break;                  
                    //creates exponent in place of elementToManipulate and returns exponent
                    Input_handler* exponent = createExponent(handler, elementToManipulate);
                    if(!exponent) break;
                    return recordInput(exponent, 1);
                }
                case Character_Sin: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, SIN_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Cos: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, COS_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Tan: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, TAN_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_InvSin: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, INVSIN_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_InvCos: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, INVCOS_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_InvTan: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, INVTAN_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_SquareRoot: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, SQRT_FUNCTION, 0, false);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Root: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, ROOT_FUNCTION, 0, false);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Log: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, LOG_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);            
                }
                case Character_LogBase: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, LOG_BASE_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);            
                }
                case Character_Ln: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, LN_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Absolute: {
                    if(exceedsMaxSize) break;
                    Input_handler* function = createFunction(handler, elementToManipulate, ABSOLUTE_FUNCTION, 0, true);
                    if(!function) break;
                    return recordInput(function, 1);
                }
                case Character_Square: {
                    if(exceedsMaxSize) break;                  
                    //creates exponent in place of elementToManipulate and returns exponent
                    Input_handler* exponent = createExponent(handler, elementToManipulate);
                    if(!exponent) break;

                    Display_char* firstCharacter = &exponent->buffer[0];
                    initializeDisplayChar(firstCharacter, handler->scale); // "2" character
                    firstCharacter->type = VARIABLE_CHARACTER;
                    firstCharacter->data.variable = '2';
                    //exponent->size++;
                    exponent->position++;

                    handler->position++;
                    break;
                }
                case Character_Reciprocal: {
                    if(exceedsMaxSize) break;                  
                    //creates exponent in place of elementToManipulate and returns exponent
                    Input_handler* exponent = createExponent(handler, elementToManipulate);
                    if(!exponent) break;

                    Display_char* firstCharacter = &exponent->buffer[0];
                    initializeDisplayChar(firstCharacter, handler->scale); // "-" character
                    firstCharacter->type = VARIABLE_CHARACTER;
                    firstCharacter->data.variable = '-';
                    exponent->size++;
                    exponent->position++;

                    Display_char* secondCharacter = &exponent->buffer[1];
                    initializeDisplayChar(secondCharacter, handler->scale); // "1" character
                    secondCharacter->type = VARIABLE_CHARACTER;
                    secondCharacter->data.variable = '1';
                    //exponent->size++;
                    exponent->position++;

                    handler->position++;

                    break;
                }
                case Character_TenPower: {

                }
                case Character_Euler: {

                }
                default:
                    if(character != '?' && !exceedsMaxSize) {
                        if(elementToManipulate->type != EMPTY_CHARACTER) {
                            if(elementToManipulate->type == GAP_CHARACTER) {
                                Input_shiftRight(handler, handler->position);
                                handler->size--;
                                elementToManipulate = &handler->buffer[handler->position];
                            } else {
                                freeCharacter(elementToManipulate);
                                handler->size--;
                            }
                        }

                        initializeDisplayChar(elementToManipulate, handler->scale);
                        elementToManipulate->type = VARIABLE_CHARACTER;
                        elementToManipulate->data.variable = character;

                        handler->size++;
                        if (Input_moveRight(handler)) return;
                    }
                    break;
            }

            setCursorAtPosition(handler, handler->position);


            gfx_SetDrawBuffer();

            Cursor.node = &handler->buffer[handler->position];
            setInputCharacterPositions(leftMostInputHandler, 0, 0);
            positionCursor(handler, handler->position, -leftMostInputHandler->maxBounds->x, -leftMostInputHandler->maxBounds->y); //for position
            renderInput(leftMostInputHandler, true, leftMostInputHandler->maxBounds->width/2 - Cursor.x, leftMostInputHandler->maxBounds->height/2 - Cursor.y - Cursor.height/2);
            renderCursor(leftMostInputHandler, leftMostInputHandler->maxBounds->width/2 - Cursor.x, leftMostInputHandler->maxBounds->height/2 - Cursor.y - Cursor.height/2, exceedsMaxSize);
            
            gfx_BlitRectangle(gfx_buffer, leftMostInputHandler->maxBounds->x, leftMostInputHandler->maxBounds->y, leftMostInputHandler->maxBounds->width, leftMostInputHandler->maxBounds->height);
            gfx_SetDrawScreen();
        } 

        //render cursor, for blinky blink
        gfx_SetDrawBuffer();
        renderCursor(leftMostInputHandler, leftMostInputHandler->maxBounds->width/2 - Cursor.x, leftMostInputHandler->maxBounds->height/2 - Cursor.y - Cursor.height/2, exceedsMaxSize);
        gfx_BlitRectangle(gfx_buffer, leftMostInputHandler->maxBounds->x, leftMostInputHandler->maxBounds->y, leftMostInputHandler->maxBounds->width, leftMostInputHandler->maxBounds->height);
        gfx_SetDrawScreen();

        previousKey = key;
    } while(key != sk_Enter);
}

void setInputCharacterPositions(Input_handler* handler, int offsetX, int offsetY) {
    if (!handler || !handler->buffer || !handler->currentBounds) return;

    // Reset bounds width for new rendering
    handler->currentBounds->width = 0;

    int aboveOriginHeight = getExpressionAboveOriginHeight(handler);
    int originY = handler->currentBounds->y + aboveOriginHeight;

    //set character positions
    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        Display_char* nextChar = index + 1 < handler->size ? &handler->buffer[index + 1] : NULL;
        if (currentChar && currentChar->type != EMPTY_CHARACTER && currentChar->setPosition) {
            currentChar->setPosition(currentChar, handler->currentBounds, originY, offsetX, offsetY);
            if(nextChar != NULL && (nextChar->type != GAP_CHARACTER && nextChar->type != EMPTY_CHARACTER && nextChar->type != CURSOR_CHARCATER) && (currentChar->type != GAP_CHARACTER && currentChar->type != EMPTY_CHARACTER && currentChar->type != CURSOR_CHARCATER)) handler->currentBounds->width += inputFieldCharacterSpacing;
        }
    }
}
void renderInput(Input_handler* handler, bool clear, int offsetX, int offsetY) {
    if (!handler || !handler->buffer || !handler->currentBounds) return;

    //if original then cear input area
    if(clear) {
        //also the furtherest left input handler
        gfx_SetColor(inputBackgroundColor);
        gfx_FillRectangle_NoClip(handler->maxBounds->x, handler->maxBounds->y, handler->maxBounds->width, handler->maxBounds->height);

        if(handler->buffer[0].type != EMPTY_CHARACTER && handler->buffer[0].type != GAP_CHARACTER) {
            int furthestVisualLeftX = handler->buffer[0].x + offsetX;
            if(furthestVisualLeftX > handler->maxBounds->x) {
                offsetX -= furthestVisualLeftX - handler->maxBounds->x; //take away the difference from the vidual x to the begining of input to align the input to begining
            }
        }
    }

    // Render characters
    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        if (currentChar->type != EMPTY_CHARACTER && currentChar->display) {
            currentChar->display(currentChar, offsetX, offsetY);
        }
    }


}
void deleteLastCursor(Input_handler* handler) {
    if(handler->size == 0) return;
    else if(handler->buffer[handler->size - 1].type == CURSOR_CHARCATER) {
        freeCharacter(&handler->buffer[handler->size - 1]);
        handler->size--;
    }
}
void setCursorAtPosition(Input_handler* handler, int position) {
    if(
        position < handler->maxSize && (
            (position == 0 && handler->buffer[position].type == EMPTY_CHARACTER) ||
            (position > 0 && handler->buffer[position - 1].type != EMPTY_CHARACTER && handler->buffer[position - 1].type != CURSOR_CHARCATER && handler->buffer[position].type == EMPTY_CHARACTER)
        ) &&
        handler->size < handler->maxSize
    ) {

        Display_char* elementToManipulate = &handler->buffer[position];

        elementToManipulate->type = CURSOR_CHARCATER;
        elementToManipulate->data.variable = '0'; //standard char
        elementToManipulate->scale = handler->scale;
        elementToManipulate->setPosition = Character_setPosition;
        elementToManipulate->display = Character_render;
        elementToManipulate->getAboveOriginHeight = Character_getAboveOriginHeight;
        elementToManipulate->getBelowOriginHeight = Character_getBelowOriginHeight;
        elementToManipulate->getWidth = Character_getWidth;
        elementToManipulate->getHeight = Character_getHeight;

        handler->size++;
    }
}
void positionCursor(Input_handler* handler, int position, int offsetX, int offsetY) {
    if (position >= handler->size) return;
    //the outline
    
    Display_char* refrenceNode = &handler->buffer[position];

    if(!refrenceNode || refrenceNode->type == EMPTY_CHARACTER) return;

    Cursor.x = refrenceNode->x + offsetX;
    Cursor.y = refrenceNode->y + offsetY;
    Cursor.width = refrenceNode->getWidth(refrenceNode);
    Cursor.height = refrenceNode->getHeight(refrenceNode);
    Cursor.node = refrenceNode;
}
void renderCursor(Input_handler* handler, int offsetX, int offsetY, bool nullCursor) {
    Display_char* refrenceNode = Cursor.node;
    if(!refrenceNode || refrenceNode->type == EMPTY_CHARACTER) return;

    int furthestVisualLeftX = handler->buffer[0].x + offsetX;
    if(furthestVisualLeftX > handler->maxBounds->x) {
        offsetX -= furthestVisualLeftX - handler->maxBounds->x; //take away the difference from the vidual x to the begining of input to align the input to begining
    }

    if(Cursor.ticker > 128) {
        //no render cursor
        // the 2 is just how much it iterates per tick
        if(Cursor.ticker - 2 <= 128) {
            //render the node, previously erased
            gfx_SetColor(inputBackgroundColor);
            gfx_FillRectangle(refrenceNode->x + offsetX, refrenceNode->y + offsetY, Cursor.width, Cursor.height);
            if(refrenceNode->type != CURSOR_CHARCATER && refrenceNode->type != GAP_CHARACTER) refrenceNode->display(refrenceNode, offsetX, offsetY);
        }

        Cursor.ticker += 2;
        return;
    }

    int width = Cursor.width;

    gfx_SetColor(inputTextColor);
    if(nullCursor) {
        gfx_SetColor(100);
    }

    gfx_FillRectangle(refrenceNode->x + offsetX, refrenceNode->y + offsetY, width, Cursor.height);
    if(refrenceNode->type == GAP_CHARACTER) refrenceNode->display(refrenceNode, offsetX, offsetY);
    else if(second && alpha && width >= 6) {
        char string[2] = {Character_SecondAlphaCursorIdentifier, '\0'}; // underlined A

        gfx_SetTextFGColor(inputBackgroundColor);
        gfx_SetTextScale(1, refrenceNode->scale); 
        gfx_PrintStringXY(string, refrenceNode->x + offsetX, refrenceNode->y + offsetY);
    }
    else if(second && width >= 6) {
        char string[2] = {Character_SecondCursorIdentifier, '\0'}; //up arrow

        gfx_SetTextFGColor(inputBackgroundColor);
        gfx_SetTextScale(1, refrenceNode->scale); 
        gfx_PrintStringXY(string, refrenceNode->x + offsetX, refrenceNode->y + offsetY);
    }
    else if(alpha && width >= 6) {
        char string[2] = {Character_AlphaCursorIdentifier, '\0'}; //an 'A'

        gfx_SetTextFGColor(inputBackgroundColor);
        gfx_SetTextScale(1, refrenceNode->scale); 
        gfx_PrintStringXY(string, refrenceNode->x + offsetX, refrenceNode->y + offsetY);
    }

    Cursor.ticker += 2;
}
int getExpressionHeight(Input_handler* handler) {
    if(!handler || !handler->buffer) return 0;
    
     int maxHeight = 0;

    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        if (currentChar->type != EMPTY_CHARACTER) {
            if (currentChar->getAboveOriginHeight) {
                maxHeight = fmax(maxHeight, 
                    currentChar->getHeight(currentChar));
            }
        }
    }

    return maxHeight;
}
int getExpressionAboveOriginHeight(Input_handler* handler) {
    if(!handler || !handler->buffer) return 0;

     int aboveOriginHeight = 0;

    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        if (currentChar->type != EMPTY_CHARACTER) {
            if (currentChar->getAboveOriginHeight) {
                aboveOriginHeight = fmax(aboveOriginHeight, 
                    currentChar->getAboveOriginHeight(currentChar));
            }
        }
    }

    return aboveOriginHeight;
}
int getExpressionBelowOriginHeight(Input_handler* handler) {
    if(!handler || !handler->buffer) return 0;

     int belowOriginHeight = 0;

    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        if (currentChar->type != EMPTY_CHARACTER) {
            if (currentChar->getBelowOriginHeight) {
                belowOriginHeight = fmax(belowOriginHeight, 
                    currentChar->getBelowOriginHeight(currentChar));
            }
        }
    }

    return belowOriginHeight;
}
int getExpressionWidth(Input_handler* handler) {
    if(!handler || !handler->buffer) return 0;
     
     int width = 0;

    for( int index = 0; index < handler->size; index++) {
        Display_char* currentChar = &handler->buffer[index];
        Display_char* nextChar = index + 1 < handler->size ? &handler->buffer[index + 1] : NULL;
        if (currentChar->type != EMPTY_CHARACTER) {
            if (currentChar->getBelowOriginHeight) {
                width += currentChar->getWidth(currentChar);
                if(nextChar != NULL && (nextChar->type != GAP_CHARACTER && nextChar->type != EMPTY_CHARACTER && nextChar->type != CURSOR_CHARCATER) && (currentChar->type != GAP_CHARACTER && currentChar->type != EMPTY_CHARACTER && currentChar->type != CURSOR_CHARCATER)) width += inputFieldCharacterSpacing;
            }
        }
    }

    return width;
}



Input_handler* createInputHandler(int maxSize) {
    Input_handler* handler = malloc(sizeof(Input_handler));
    if (!handler) return NULL;

    handler->size = 0;
    handler->position = 0;
    handler->maxSize = maxSize;
    handler->scale = 1;
    handler->capacity = fmin(INITIAL_CAPACITY, maxSize); //ininital capacity

    handler->buffer = calloc(handler->capacity, sizeof(Display_char));
    if (!handler->buffer) {
        free(handler);
        return NULL;
    }

    // Initialize all display functions for empty characters
    for( int i = 0; i < handler->maxSize; i++) {
        handler->buffer[i].type = EMPTY_CHARACTER;
        handler->buffer[i].display = NULL;
        handler->buffer[i].setPosition = NULL;
        handler->buffer[i].getAboveOriginHeight = NULL;
        handler->buffer[i].getBelowOriginHeight = NULL;
        handler->buffer[i].getWidth = NULL;
        handler->buffer[i].getHeight = NULL;
    }

    // Add bounds initialization
    handler->currentBounds = malloc(sizeof(Bounds_char));
    if (!handler->currentBounds) {
        free(handler->buffer);
        free(handler);
        return NULL;
    }
    handler->currentBounds->x = 0;
    handler->currentBounds->y = 0;
    handler->currentBounds->width = 0;
    handler->currentBounds->height = 0;

    handler->maxBounds = malloc(sizeof(Bounds_char));
    if (!handler->maxBounds) {
        free(handler->currentBounds);
        free(handler->buffer);
        free(handler);
        return NULL;
    }
    handler->maxBounds->x = 0;
    handler->maxBounds->y = 0;
    handler->maxBounds->width = GFX_LCD_WIDTH;
    handler->maxBounds->height = GFX_LCD_HEIGHT;

    handler->left = NULL;
    handler->right = NULL;

    return handler;
}

bool resizeBuffer(Input_handler* handler, int extraCapacity) {
    if (!handler || extraCapacity <= 0) return false;

    int newCapacity = fmin(handler->capacity + extraCapacity, handler->maxSize);
    if (newCapacity == handler->capacity) return true; //at max capacity

    Display_char* newBuffer = realloc(handler->buffer, newCapacity * sizeof(Display_char));
    if (!newBuffer) {
        // allocation failure
        return false;
    }

    // Zero-initialize the newly allocated portion
    for (int i = handler->capacity; i < newCapacity; i++) {
        newBuffer[i].type = EMPTY_CHARACTER;
        newBuffer[i].display = NULL;
        newBuffer[i].setPosition = NULL;
        newBuffer[i].getAboveOriginHeight = NULL;
        newBuffer[i].getBelowOriginHeight = NULL;
        newBuffer[i].getWidth = NULL;
        newBuffer[i].getHeight = NULL;
    }

    handler->buffer = newBuffer;
    handler->capacity = newCapacity;

    return true;
}

void freeFunction(Function_char* function) {
    if (function) {
        /*if (function->input != NULL) {
            freeInputHandler(function->input);
        }
        if(function->baseInput != NULL) {
            freeInputHandler(function->baseInput);
        }*/
        free(function);
    }
}

void freeExponent(Exponent_char* exponent) {
    if (exponent) {
        if (exponent->exponent != NULL) {
            freeInputHandler(exponent->exponent);
        }
        free(exponent);
    }
}

void freeFraction(Fraction_char* fraction) {
    if (!fraction) return;

    /*if (fraction->numerator != NULL) {
        // Clear links before freeing to prevent circular references
        freeInputHandler(fraction->numerator);
        fraction->numerator = NULL;  // Prevent dangling pointer
    }
    
    if (fraction->denominator != NULL) {
        // Clear links before freeing to prevent circular references
        freeInputHandler(fraction->denominator);
        fraction->denominator = NULL;  // Prevent dangling pointer
    }*/
    
    free(fraction);
}

void freeCharacter(Display_char* character) {
    if (!character) return;
    
    // Store the original type before clearing
    CharacterType originalType = character->type;
    
    // Free any dynamically allocated data based on type
    switch(originalType) {
        case FRACTION_CHARACTER:
            if (character->data.fraction) {
                freeFraction(character->data.fraction);
                character->data.fraction = NULL;  // Prevent dangling pointer
            }
            break;
            
        case FUNCTION_CHARACTER:
            if (character->data.function) {
                freeFunction(character->data.function);
                character->data.function = NULL;  // Prevent dangling pointer
            }
            break;
            
        case EXPONENT_CHARACTER:
            if (character->data.exponent) {
                freeExponent(character->data.exponent);
                character->data.exponent = NULL;  // Prevent dangling pointer
            }
            break;
            
        case VARIABLE_CHARACTER:
        case EMPTY_CHARACTER:
        case CURSOR_CHARCATER:
        case GAP_CHARACTER:
        case PLACEHOLDER_CHARACTER:
            break;  // No dynamic memory to free
    }
    
    // Reset all fields to a known good state
    character->type = EMPTY_CHARACTER;
    character->scale = 1;
    character->x = 0;
    character->y = 0;
    character->display = NULL;
    character->getAboveOriginHeight = NULL;
    character->getBelowOriginHeight = NULL;
    character->getWidth = NULL;
    character->getHeight = NULL;
    character->left = NULL;
    character->right = NULL;
    
    // Clear the union to prevent any residual data
    memset(&character->data, 0, sizeof(character->data));
}

void freeInputHandler(Input_handler* handler) {
    if (!handler) return;
    
    // First clear any links to prevent circular references
    handler->left = NULL;
    handler->right = NULL;
    
    // Free all characters in the buffer
    if (handler->buffer) {
        for (int i = 0; i < handler->maxSize; i++) {  // Changed from handler->size to handler->maxSize
            freeCharacter(&handler->buffer[i]);
        }
        free(handler->buffer);
        handler->buffer = NULL;  // Prevent dangling pointer
    }

    if (handler->currentBounds) {
        free(handler->currentBounds);
        handler->currentBounds = NULL;  // Prevent dangling pointer
    }
    
    if (handler->maxBounds) {
        free(handler->maxBounds);
        handler->maxBounds = NULL;  // Prevent dangling pointer
    }
    
    free(handler);
}

void clearCharacter(Display_char* character) {
    if (!character) return;
    freeCharacter(character);
}