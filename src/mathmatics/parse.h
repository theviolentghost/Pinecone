#ifndef PARSE_H
#define PARSE_H

#include "AST.h"
#include "../input.h"

Node* InputHandlerToAbstractSyntaxTree(InputHandler* handler);
InputHandler* AbstractSyntaxTreeToInputHandler(Node* node);
Node* parseTerm(InputHandler* handler, int* index);
Node* parseNumber(InputHandler* handler, int* index);
bool canParseDisplayCharacter(DisplayCharacter* character);
bool operatorHasNULLpointers(Node* node);

#endif // PARSE_H