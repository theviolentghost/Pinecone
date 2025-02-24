#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../input.h"
#include "AST.h"
#include "../global.h"
#include "parse.h" 

Node* InputHandlerToAbstractSyntaxTree(InputHandler* handler) {
    Node* root = NULL;
    Node** currentPointer = root; // points to the latest node or open branch'
    //Node* currentPointerParent = NULL; // points to the parent of the currentPointer

    int index = 0;
    while (index < handler->size) {
        Node* term = parseTerm(handler, &index); // parseTerm consumes characters

        if(!term) return NULL; // invalid term, invalid input
        
        switch(term->type) {
            case CONSTANT_NODE:
            case VARIABLE_NODE: {
                
                *currentPointer = term;
                break;
            }
            case BINARY_NODE: {
                switch(term->data.operator->operator) {
                    case ADDITION:
                    case SUBTRACTION: {
                        term->data.operator->left = root; 
                        root = term;
                        currentPointer = term->data.operator->right; // NULL

                        break;
                    }
                    case MULTIPLICATION:
                    case DIVISION:
                    case EXPONENT: {
                        // Attach the current subtree as the left child of the new operator.
                        // term->data.operator->left = currentPointer;
                        // currentPointer = term->data.operator->right; // NULL

                        // if(currentPointerParent) currentPointerParent->data.operator->right = term;
                        // currentPointerParent = term;
                        // break;
                    }
                    default: {
                        break;
                    }
                }
            }
            default:
                break;
        }
    }

    return root;
}

InputHandler* AbstractSyntaxTreeToInputHandler(Node* node) {
     return NULL;
}

bool canParseDisplayCharacter(DisplayCharacter* character) {
    return !character || !(
        character->type == GAP_CHARACTER ||
        character->type == EMPTY_CHARACTER
    );
}
bool isDigit(char character) {
    return character >= '0' && character <= '9';
}
int characterToDigit(char character) {
    if(character == '.') return 10;
    return character - '0';
}
bool isOperator(char character) {
    return character == '+' || character == '-' || character == '*' || character == '/' || character == '^';
}
OperationType characterToOperator(char character) {
    switch(character) {
        case '+': return ADDITION;
        case '-': return SUBTRACTION;
        case '*': return MULTIPLICATION;
        case '/': return DIVISION;
        case '^': return EXPONENT;
        default: return ADDITION;
    }
}

Node* parseTerm(InputHandler* handler, int* index) {
    switch(handler->buffer[*index].type) {
        case CHARACTER_CHARACTER: {
            if(isDigit(handler->buffer[*index].data.character) || handler->buffer[*index].data.character == '.') {
                return parseNumber(handler, index);
            }

            if(isOperator(handler->buffer[*index].data.character)) {
                // returns empty operator
                return Operator(characterToOperator(handler->buffer[(*index)++].data.character), NULL, NULL);
            }

            // defualt, return a variable
            return Variable(handler->buffer[(*index)++].data.character);
        }
        default: 
            return NULL;
    }
}

Node* parseNumber(InputHandler* handler, int* index) {
    float number = 0;
    bool isDecimal = false;
    int decimalPlace = 1;

    while (*index < handler->size) {
        DisplayCharacter* character = &handler->buffer[*index];

        if (isDigit(character->data.character) || character->data.character == '.') {
            int digit = characterToDigit(character->data.character);

            if (digit == 10) { // handle decimal point
                if (isDecimal) return NULL; // multiple decimals found
                isDecimal = true;
                (*index)++;
                continue;
            }

            if (isDecimal) {
                number += digit / pow(10, decimalPlace++);
            } else {
                number = number * 10 + digit;
            }

            (*index)++;
        } 
        else {
            break;
        }
    }

    return Constant(number);
}