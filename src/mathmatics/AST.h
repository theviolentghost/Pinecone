#ifndef AST_H
#define AST_H

#include "variableMap.h"
#include "../input.h"

typedef enum {
    NULL_NODE,
    BINARY_NODE,
    CONSTANT_NODE,
    VARIABLE_NODE,
    FUNCTION_NODE,
} NodeType;

typedef enum {
    ADDITION,
    SUBTRACTION,
    MULTIPLICATION,
    DIVISION,
    EXPONENT,
} OperationType;

typedef enum {
    SUCCESS,
    DIVIDE_BY_ZERO,
    UNDEFINED,
    IMAGINARY,
    INVALID_ARGUMENT,
    INVALID_OPERATOR,
    INVALID_FUNCTION,
    INVALID_VARIABLE,
    INVALID_CONSTANT,
    INVALID_NODE,
    INVALID_MAP,
    INVALID_KEY,
    INVALID_VALUE,
} ErrorCode;

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
    OperationType operator;
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
    FunctionName name;
    Node* argument;
};

Node* Constant(float value);
Node* Variable(char variable);
Node* Function(FunctionName function, Node* argument);
Node* Operator(OperationType operator, Node* left, Node* right);
void freeNode(Node* node);
float evaluateNode(Node* rootNode, VariableMap* variables, ErrorCode* error);
float Function_evaluate(FunctionName function, Node* argument, VariableMap* variables, ErrorCode* error);
float Function_baseEvaluate(FunctionName function, Node* base, Node* argument, VariableMap* variables, ErrorCode* error);
float Binary_evaluate(Node* rootNode, VariableMap* variables, ErrorCode* error);

#endif // AST_H