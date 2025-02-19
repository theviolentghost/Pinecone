#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdlib.h>

#include "../input.h"

typedef enum {
    NULL_NODE,
    BINARY_NODE,
    CONSTANT_NODE,
    VARIABLE_NODE,
    FUNCTION_NODE,
} NodeType;

struct VariableMap

typedef struct Node Node;
typedef struct OperatorNode OperatorNode;
typedef struct ConstantNode ConstantNode;
typedef struct VariableNode VariableNode;
typedef struct FunctionNode FunctionNode;

struct Node {
    NodeType type;
    union {
        struct OperatorNode* operator;
        struct ConstantNode* constant;
        struct VariableNode* variable;
        struct FunctionNode* function;
    } data;
};

struct OperatorNode {
    char operator;
    Node* left;
    Node* right;
};

struct ConstantNode {
    float value;
};

struct VariableNode {
    char variable;
};

struct FunctionNode {
    FunctionName function;
    Node* argument;
};

#endif // AST_H