#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include "./AST.h"
#include "../input.h"
#include "AST.h"
#include "../global.h"

Node* InputHandlerToAbstractSyntaxTree(InputHandler* handler) {
    Node* root = NULL;
    Node* current = NULL; // points to the latest node or open branch

    // Using a for-loop to traverse the input characters.
    for (int index = 0; index < handler->size; index++) {
        DisplayCharacter* character = &handler->buffer[index];

        if (!canParseDisplayCharacter(character)) continue; // skip this character

        Node* term = parseTerm(handler, &index); // parseTerm consumes characters
        if (term == NULL) return NULL; // error occurred

        if (root == NULL) {
            // this is the first term, so it becomes our root
            root = term;
            current = root;
        }
        else {
            // If term is an operator (i.e. its children are NULL)
            if (operatorHasNULLpointers(term)) {
                switch (term->data.operator->operator) {
                    case ADDITION:
                    case SUBTRACTION:
                        // attach the entire tree as left child
                        term->data.operator->left = root;
                        root = term;
                        // right child becomes the new open branch
                        current = term->data.operator->right;
                        break;
                    case MULTIPLICATION:
                    case DIVISION:
                    case EXPONENT: {
                        // attach the current open branch as left child
                        term->data.operator->left = current;
                        // update current to be operator's right child
                        current = term->data.operator->right;
                        break;
                    }
                    default:
                        break;
                }
            } 
            else {
                // if term is not an operator, assume implied multiplication:
                Node* multNode = Operator(MULTIPLICATION, root, term);
                root = multNode;
                // the open branch is the right child of the multiplication node.
                current = multNode->data.operator->right;
            }
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
        character->type == EMPTY_CHARACTER || 
        character->type == PLACEHOLDER_CHARACTER 
    );
}

bool isDigit(char character) {
    return (character >= '0' && character <= '9');
}
bool isOperator(char character) {
    return (character == '+' || character == '-' || character == '*' || character == '/' || character == '^');
}
bool operatorHasNULLpointers(Node* node) {
    return node->type == BINARY_NODE && (!node->data.operator->left || !node->data.operator->right);
}
bool operatorHasNULLleftPointer(Node* node) {
    return !node->data.operator->left;
}
bool operatorHasNULLrightPointer(Node* node) {
    return !node->data.operator->right;
}

int characterToDigit(char character) {
    //assumes isDigit == true 
    if(character == '.') return 10;
    return character - '0';
}

Node* parseTerm(InputHandler* handler, int* index) {
    switch(handler->buffer[*index].type) {
        case CHARACTER_CHARACTER: {
            if(isDigit(handler->buffer[*index].data.character) || handler->buffer[*index].data.character == '.') {
                return parseNumber(handler, index);
            }

            if(isOperator(handler->buffer[*index].data.character)) {
                return Operator(handler->buffer[*index].data.character, NULL, NULL);
            }

            // character is a variable/symbol 'a', doesn't do strings: "abc"
            return Variable(handler->buffer[(*index)++].data.character); // move to next character
        }
        case FUNCTION_CHARACTER: {
            DisplayCharacter* functionCharacter = &handler->buffer[(*index)++]; // move to next character
            Node* argument = InputHandlerToAbstractSyntaxTree(functionCharacter->input);
            if(!argument) return NULL;

            return Function(functionCharacter->name, argument);
        }
        case FRACTION_CHARACTER: {
            Node* numerator = InputHandlerToAbstractSyntaxTree(handler->buffer[*index].data.fraction->numerator);
            Node* denominator = InputHandlerToAbstractSyntaxTree(handler->buffer[*index].data.fraction->denominator);

            (*index)++; // move to next character

            if(!numerator || !denominator) return NULL;

            return Binary('/', numerator, denominator);
        }
        case EXPONENT_CHARACTER: {
            Node* exponent = InputHandlerToAbstractSyntaxTree(handler->buffer[(*index)++].data.exponent->exponent);
            if(!exponent) return NULL;

            return exponent;
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
            // should also account for numbers such as .0001 instead of 0.0001
            if(isDecimal) return NULL; // multiple decimal points ..01.01
            isDecimal = true;
            (*index)++; // move to next character
            continue;
        }

        if(!isDecimal) number *= 10; // shift left

        
        number += isDecimal ? ((float)digit / powf(10, decimalPlace++)) : digit;

        (*index)++;
    } while((isDigit(handler->buffer[*index].data.character) || handler->buffer[*index].data.character == '.') && *index < handler->size); //include decimal point

    return Constant(number); // return new constant node of number
}