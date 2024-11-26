#include <stdbool.h>

#include "../input.h"

typedef enum {
    NODE_CONSTANT,
    NODE_VARIABLE,
    NODE_VARIABLE_FLAT, // Coefficient and exponent represented as doubles instead of nodes
    NODE_FUNCTION,
    NODE_FUNCTION_FLAT, 
    NODE_OPERATOR
} NodeType;

typedef enum {
    OPERATION_ADD,
} OperationType;

struct Constant {
    double value;
};
struct Variable {
    char name;
    // add derivative support
    Node* coefficient;
    Node* exponent;
};
struct VariableFlat {
    char name;
    double coefficient;
    double exponent;
};
struct Function {
    FunctionName name;
    Node* input;
    float base;
    Node* coefficient;
    Node* exponent;
};
struct FunctionFlat {
    FunctionName name;
    Node* input;
    float base;
    double coefficient;
    double exponent;
};
struct Operator {

}

struct Node {
    NodeType type;

    // type structs
    union {
        
    };
};
