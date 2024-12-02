#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../input.h"
#include "AST.h"
#include "../global.h"

Node* InputToAST(Input_handler* handler);
Input_handler* ASTToInput(Node* node);

#endif