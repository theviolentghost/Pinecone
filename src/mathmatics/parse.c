#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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
    return character >= '0' && character <= '9';
}
int characterToDigit(char character) {
    //assumes isDigit == true
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
    int number = 0;

    do {
        number *= 10; // shift left
        number += characterToDigit(handler->buffer[*index].data.character);

        (*index)++;
    } while(isDigit(handler->buffer[*index].data.character));

    return NULL; // return new constant node of number
}