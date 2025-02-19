#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include "AST.h"
#include "../input.h"
#include "AST.h"
#include "../global.h"

Node* InputHandlerToAbstractSyntaxTree(InputHandler* handler) {
     for(int index = 0; index < handler->size; index++) {
        DisplayCharacter* character = &handler->buffer[index];

        if(!canParseDisplayCharacter(character)) continue;

        Node* term = parseTerm(handler, &index);
     }
}

InputHandler* AbstractSyntaxTreeToInputHandler(Node* node) {
     return NULL;
}

bool canParseDisplayCharacter(DisplayCharacter* character) {
    return !character || (
        character->type == GAP_CHARACTER ||
        character->type == EMPTY_CHARACTER
    );
}

bool isDigit(char character) {
    return (character >= '0' && character <= '9');
}

int characterToDigit(char character) {
    //assumes isDigit == true 
    if(character == '.') return 10;
    return character - '0';
}

Node* parseTerm(InputHandler* handler, int* index) {
    switch(handler->buffer[*index].type) {
        case CHARACTER_CHARACTER: {
            if(isDigit(handler->buffer[*index].data.character)) {
                return parseNumber(handler, index);
            }

            // character is a variable/symbol 'a'

        }
    }
}

Node* parseNumber(InputHandler* handler, int* index) {
    //assumes first character is a digit
    float number = 0;
    bool isDecimal = false;
    int decimalPlace = 1;

    do {
        int digit = characterToDigit(handler->buffer[*index].data.character);
        if(digit == 10) {
            if(isDecimal) return NULL; // multiple decimal points
            isDecimal = true;
            continue;
        }

        if(!isDecimal) number *= 10; // shift left

        
        number += isDecimal ? (digit(float) / powf(10, decimalPlace++)) : digit;

        (*index)++;
    } while(isDigit(handler->buffer[*index].data.character) || handler->buffer[*index].data.character == '.'); //include decimal point

    return NULL; // return new constant node of number
}