#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "../input.h"
#include "AST.h"
#include "../global.h"
#include "variableMap.h"

Node* Constant(float value) {
    Node* node = malloc(sizeof(Node));

    if(!node) return NULL;
    node->type = CONSTANT_NODE;

    node->data.constant = malloc(sizeof(ConstantNode));
    if(!node->data.constant) {
        free(node);
        return NULL;
    }
    node->data.constant->value = value;

    return node;
}

Node* Variable(char variable) {
    Node* node = malloc(sizeof(Node));

    if(!node) return NULL;
    node->type = VARIABLE_NODE;

    node->data.variable = malloc(sizeof(VariableNode));
    if(!node->data.variable) {
        free(node);
        return NULL;
    }
    node->data.variable->variable = variable;

    return node;
}

Node* Function(FunctionName function, Node* argument) {
    Node* node = malloc(sizeof(Node));

    if(!node) return NULL;
    node->type = FUNCTION_NODE;

    node->data.function = malloc(sizeof(FunctionNode));
    if(!node->data.function) {
        free(node);
        return NULL;
    }
    node->data.function->name = function;
    node->data.function->argument = argument;

    return node;
}

Node* Operator(OperationType operator, Node* left, Node* right) {
    Node* node = malloc(sizeof(Node));

    if(!node) return NULL;
    node->type = BINARY_NODE;

    node->data.operator = malloc(sizeof(OperatorNode));
    if(!node->data.operator) {
        free(node);
        return NULL;
    }
    node->data.operator->operator = operator;
    node->data.operator->left = left;
    node->data.operator->right = right;

    return node;
}

void freeNode(Node* node) {
    if(!node) return;

    switch(node->type) {
        case CONSTANT_NODE:
            free(node->data.constant);
            break;
        case VARIABLE_NODE:
            free(node->data.variable);
            break;
        case FUNCTION_NODE:
            freeNode(node->data.function->argument);
            free(node->data.function);
            break;
        case BINARY_NODE:
            freeNode(node->data.operator->left);
            freeNode(node->data.operator->right);
            free(node->data.operator);
            break;
        default:
            break;
    }

    free(node);
}

///////////////////////////////////////////////////////////////////
// tree manipulation functions

float evaluateNode(Node* rootNode, VariableMap* variables, ErrorCode* error) {
    if(!rootNode) return 0.0;
    float value = 0.0;

    switch(rootNode->type) {
        case CONSTANT_NODE:
            return rootNode->data.constant->value;
        case VARIABLE_NODE:
            value = variableMapGet(variables, rootNode->data.variable->variable);
            if(value == NAN) {
                *error = INVALID_VARIABLE;
                return 0.0;
            }
            break;
        case FUNCTION_NODE:
            value = Function_evaluate(rootNode, rootNode->data.function->argument, variables, error);
            break;
        case BINARY_NODE:
            value = Binary_evaluate(rootNode, variables, error);
            break;
        default:
            *error = INVALID_NODE;
    }

    return value;
     
}

float Function_evaluate(FunctionName function, Node* argument, VariableMap* variables, ErrorCode* error) {
    if(!argument) {
        *error = INVALID_ARGUMENT;
        return 0.0;
    }

    float argumentValue = evaluateNode(argument, variables, error);
    if(*error != SUCCESS) return 0.0; // error occurred

    switch(function) {
        case SIN_FUNCTION:
            return sinf(argumentValue);
        case COS_FUNCTION:
            return cosf(argumentValue);
        case TAN_FUNCTION:
            return tanf(argumentValue);
        case CSC_FUNCTION: {
            float denominator = sinf(argumentValue);
            if(denominator == 0) {
                *error = UNDEFINED;
                return 0.0;
            }
            return 1.0 / denominator;
        }
        case SEC_FUNCTION: {
            float denominator = cosf(argumentValue);
            if(denominator == 0) {
                *error = UNDEFINED;
                return 0.0;
            }
            return 1.0 / denominator;
        }
        case COT_FUNCTION: {
            float denominator = tanf(argumentValue);
            if(denominator == 0) {
                *error = UNDEFINED;
                return 0.0;
            }
            return 1.0 / denominator;
        }
        case INVSIN_FUNCTION:
            return asinf(argumentValue);
        case INVCOS_FUNCTION:
            return acosf(argumentValue);
        case INVTAN_FUNCTION:
            return atanf(argumentValue);
        case INVCSC_FUNCTION:
            return asinf(1.0 / argumentValue);
        case INVSEC_FUNCTION:
            return acosf(1.0 / argumentValue);
        case INVCOT_FUNCTION:    
            return atanf(1.0 / argumentValue);
        case SQRT_FUNCTION:
            if(argumentValue < 0) {
                *error = IMAGINARY;
                return 0.0;
            }
            return sqrtf(argumentValue);
        case ABSOLUTE_FUNCTION:
            return fabsf(argumentValue);
        case LOG_FUNCTION:
        case LN_FUNCTION:
            if(argumentValue <= 0) {
                *error = INVALID_ARGUMENT;
                return 0.0;
            }
            return logf(argumentValue);
        case LOG_10_FUNCTION:
            if(argumentValue <= 0) {
                *error = INVALID_ARGUMENT;
                return 0.0;
            }
            return log10f(argumentValue);
        default:
            *error = INVALID_FUNCTION;
            return 0.0;
    }
}

float Function_baseEvaluate(FunctionName function, Node* base, Node* argument, VariableMap* variables, ErrorCode* error) {
    // for functions that have a base: logs, roots
    if(!base || !argument) {
        *error = INVALID_ARGUMENT;
        return 0.0;
    }

    float baseValue = evaluateNode(base, variables, error);
    if(*error != SUCCESS) return 0.0; // error occurred

    float argumentValue = evaluateNode(argument, variables, error);
    if(*error != SUCCESS) return 0.0; // error occurred

    switch(function) {
        case LOG_BASE_FUNCTION:
            if(baseValue <= 0 || baseValue == 1) {
                *error = INVALID_ARGUMENT;
                return 0.0;
            }
            if(argumentValue <= 0) {
                *error = INVALID_ARGUMENT;
                return 0.0;
            }
            return logf(argumentValue) / logf(baseValue);
        case ROOT_FUNCTION:
            if(baseValue <= 0 || baseValue == 1) {
                *error = INVALID_ARGUMENT;
                return 0.0;
            }
            if(argumentValue < 0) {
                *error = IMAGINARY;
                return 0.0;
            }
            return powf(argumentValue, 1.0 / baseValue);
        default:
            *error = INVALID_FUNCTION;
            return 0.0;
    }
}

float Binary_evaluate (Node* rootNode, VariableMap* variables, ErrorCode* error) {
    if(!rootNode) {
        *error = INVALID_NODE;
        return 0.0;
    }
    if(rootNode->type != BINARY_NODE) {
        *error = INVALID_NODE;
        return 0.0;
    }

    float leftValue = evaluateNode(rootNode->data.operator->left, variables, error);
    if(*error != SUCCESS) return 0.0; // error occurred

    float rightValue = evaluateNode(rootNode->data.operator->right, variables, error);
    if(*error != SUCCESS) return 0.0; // error occurred

    switch(rootNode->data.operator->operator) {
        case ADDITION:
            return leftValue + rightValue;
        case SUBTRACTION:
            return leftValue - rightValue;
        case MULTIPLICATION:
            return leftValue * rightValue;
        case DIVISION:
            if(rightValue == 0) {
                *error = DIVIDE_BY_ZERO;
                return 0.0;
            }
            return leftValue / rightValue;
        case EXPONENT:
            return powf(leftValue, rightValue);
        default:
            *error = INVALID_OPERATOR;
            return 0.0;
    }
}


///////////////////////////////////////////////////////////////////