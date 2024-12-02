#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../input.h"
#include "AST.h"
#include "../global.h"

VariableLookup* createVariableLookup() {
    VariableLookup* lookup = (VariableLookup*)malloc(sizeof(VariableLookup));
    if (lookup) {
        memset(lookup->buckets, 0, sizeof(lookup->buckets));
        lookup->size = HASH_TABLE_SIZE;
    }
    return lookup;
}

void destroyVariableLookup(VariableLookup* lookup) {
    if (!lookup) return;
    for (size_t i = 0; i < lookup->size; i++) {
        VariableEntry* current = lookup->buckets[i];
        while (current) {
            VariableEntry* next = current->next;
            free(current->name);
            free(current);
            current = next;
        }
    }
    free(lookup);
}

VariableEntry* findVariableEntry(VariableLookup* lookup, const char* name) {
    if (!lookup || !name) return NULL;
    size_t index = hash(name) % lookup->size;
    VariableEntry* current = lookup->buckets[index];
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void insertVariableEntry(VariableLookup* lookup, const char* name, double value) {
    if (!lookup || !name) return;
    size_t index = hash(name) % lookup->size;
    VariableEntry* new_entry = (VariableEntry*)malloc(sizeof(VariableEntry));
    if (new_entry) {
        new_entry->name = strdup(name);
        new_entry->value = value;
        new_entry->next = lookup->buckets[index];
        lookup->buckets[index] = new_entry;
    }
}

size_t hash(const char* str) {
    size_t hash = 0;
    while (*str) {
        hash = (hash * 31) + *str++;
    }
    return hash;
}

struct Node {
    NodeType type;

    // type structs
    union {
        struct ConstantNode constant;
        struct VariableNode variable;
        struct VariableFlatNode variableFlat;
        struct FunctionNode function;
        struct FunctionFlatNode functionFlat;
        struct OperatorNode operator;
    } data;

    double (*evaluate)(Node*);
    float (*evaluateFloat)(Node*);
    Node* (*derivative)(Node*, char);
    Node* (*antiderivative)(Node*, char);
};

VariableEntry* Variables;
// VariableEntry* Variables = createVariableLookup();

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Util

Node* solveSymbolic(Node* tree, char* variable) {
    if (!tree) return NULL;
    if(tree->type != NODE_OPERATOR) return NULL;
    if(tree->data.operator.operation != OPERATION_EQUAL) return NULL;
    
    Node* left = tree->data.operator.left;
    Node* right = tree->data.operator.right;

    
}

bool containsVariable(Node* node, char* variable) {
    if(!node) return false;

    if(node->type == NODE_VARIABLE || node->type == NODE_VARIABLE_FLAT) {
        if(strcmp(node->data.variable.name, variable) == 0) return true;
        else return false;
    }
    if(node->type == NODE_FUNCTION || node->type == NODE_FUNCTION_FLAT) {
        return containsVariable(node->data.function.input, variable);
    }
    if(node->type == NODE_OPERATOR) {
        return containsVariable(node->data.operator.left, variable) || containsVariable(node->data.operator.right, variable);
    }

    return false;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Constant

Node* Constant(double value) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_CONSTANT;
        node->data.constant.value = value;
        node->evaluate = Constant_evaluate;
        node->evaluateFloat = Constant_evaluateFloat;
        node->derivative = Constant_derivative;
        node->antiderivative = Constant_antiderivative;
    }
    return node;
}
double Constant_evaluate(Node* node) {
    return node->data.constant.value;
}
float Constant_evaluateFloat(Node* node) {
    return (float)node->data.constant.value;
}
Node* Constant_derivative(Node* node, char var) {
    return Constant(0);
}
Node* Constant_antiderivative(Node* node, char var) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Variable

Node* Variable(char name) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_VARIABLE;
        node->data.variable.name = name;
        node->data.variable.coefficient = Constant(1);
        node->data.variable.exponent = Constant(1);
        node->evaluate = Variable_evaluate;
        node->evaluateFloat = Variable_evaluateFloat;
        node->derivative = Variable_derivative;
        node->antiderivative = Variable_antiderivative;
    }
    return node;
}
double Variable_evaluate(Node* node) {
    double coefficient = node->data.variable.coefficient->evaluate(node->data.variable.coefficient);
    double exponent = node->data.variable.exponent->evaluate(node->data.variable.exponent);
    double variableValue = findVariableEntry(Variables, node->data.variable.name)->value;
    return coefficient * pow(variableValue, exponent);
}
float Variable_evaluateFloat(Node* node) {
    float coefficient = node->data.variable.coefficient->evaluateFloat(node->data.variable.coefficient);
    float exponent = node->data.variable.exponent->evaluateFloat(node->data.variable.exponent);
    float variableValue = (float)findVariableEntry(Variables, node->data.variable.name)->value;
    return coefficient * powf(variableValue, exponent);
}
Node* Variable_derivative(Node* node, char var) {
    return NULL;
}
Node* Variable_antiderivative(Node* node, char var) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// VariableFlat

Node* VariableFlat(char name) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_VARIABLE_FLAT;
        node->data.variableFlat.name = name;
        node->data.variableFlat.coefficient = 1;
        node->data.variableFlat.exponent = 1;
        node->evaluate = VariableFlat_evaluate;
        node->evaluateFloat = VariableFlat_evaluateFloat;
        node->derivative = VariableFlat_derivative;
        node->antiderivative = VariableFlat_antiderivative;
    }
    return node;
}
double VariableFlat_evaluate(Node* node) {
    return node->data.variableFlat.coefficient * pow(node->data.variableFlat.name, node->data.variableFlat.exponent);
}
float VariableFlat_evaluateFloat(Node* node) {
    return (float)node->data.variableFlat.coefficient * powf(node->data.variableFlat.name, node->data.variableFlat.exponent);
}
Node* VariableFlat_derivative(Node* node, char var) {
    return NULL;
}
Node* VariableFlat_antiderivative(Node* node, char var) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Function

Node* Function(FunctionName name, Node* input, Node* base) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_FUNCTION;
        node->data.function.name = name;
        node->data.function.input = input;
        node->data.function.base = base;
        node->data.function.coefficient = Constant(1);
        node->data.function.exponent = Constant(1);
        switch(name) {
            case SIN_FUNCTION:
                node->evaluate = Function_Sin_evaluate;
                node->evaluateFloat = Function_Sin_evaluateFloat;
                break;
            case COS_FUNCTION:
                node->evaluate = Function_Cos_evaluate;
                node->evaluateFloat = Function_Cos_evaluateFloat;
                break;
            case TAN_FUNCTION:
                node->evaluate = Function_Tan_evaluate;
                node->evaluateFloat = Function_Tan_evaluateFloat;
                break;
        }
        node->derivative = Function_derivative;
        node->antiderivative = Function_antiderivative;
    }
    return node;
}
double Function_Sin_evaluate(Node* node) { node->data.function.coefficient->evaluate(node->data.function.coefficient) * pow(sin(node->data.function.input->evaluate(node->data.function.input)), node->data.function.exponent->evaluate(node->data.function.exponent)); }
double Function_Cos_evaluate(Node* node) { node->data.function.coefficient->evaluate(node->data.function.coefficient) * pow(cos(node->data.function.input->evaluate(node->data.function.input)), node->data.function.exponent->evaluate(node->data.function.exponent)); }
double Function_Tan_evaluate(Node* node) { node->data.function.coefficient->evaluate(node->data.function.coefficient) * pow(tan(node->data.function.input->evaluate(node->data.function.input)), node->data.function.exponent->evaluate(node->data.function.exponent)); }

float Function_Sin_evaluateFloat(Node* node) { node->data.function.coefficient->evaluateFloat(node->data.function.coefficient) * powf(sinf(node->data.function.input->evaluateFloat(node->data.function.input)), node->data.function.exponent->evaluateFloat(node->data.function.exponent)); }
float Function_Cos_evaluateFloat(Node* node) { node->data.function.coefficient->evaluateFloat(node->data.function.coefficient) * powf(cosf(node->data.function.input->evaluateFloat(node->data.function.input)), node->data.function.exponent->evaluateFloat(node->data.function.exponent)); }
float Function_Tan_evaluateFloat(Node* node) { node->data.function.coefficient->evaluateFloat(node->data.function.coefficient) * powf(tanf(node->data.function.input->evaluateFloat(node->data.function.input)), node->data.function.exponent->evaluateFloat(node->data.function.exponent)); }

Node* Function_derivative(Node* node, char var) {
    return NULL;
}
Node* Function_antiderivative(Node* node, char var) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// FunctionFlat

Node* FunctionFlat(FunctionName name, Node* input, float base) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_FUNCTION_FLAT;
        node->data.functionFlat.name = name;
        node->data.functionFlat.input = input;
        node->data.functionFlat.base = base;
        node->data.functionFlat.coefficient = 1;
        switch(name) {
            case SIN_FUNCTION:
                node->evaluate = FunctionFlat_Sin_evaluate;
                node->evaluateFloat = FunctionFlat_Sin_evaluateFloat;
                break;
            case COS_FUNCTION:
                node->evaluate = FunctionFlat_Cos_evaluate;
                node->evaluateFloat = FunctionFlat_Cos_evaluateFloat;
                break;
            case TAN_FUNCTION:
                node->evaluate = FunctionFlat_Tan_evaluate;
                node->evaluateFloat = FunctionFlat_Tan_evaluateFloat;
                break;
        }
        node->derivative = FunctionFlat_derivative;
        node->antiderivative = FunctionFlat_antiderivative;
    }
    return node;
}
double FunctionFlat_Sin_evaluate(Node* node) { node->data.functionFlat.coefficient * pow(sin(node->data.functionFlat.input->evaluate(node->data.functionFlat.input)), node->data.functionFlat.exponent); }
double FunctionFlat_Cos_evaluate(Node* node) { node->data.functionFlat.coefficient * pow(cos(node->data.functionFlat.input->evaluate(node->data.functionFlat.input)), node->data.functionFlat.exponent); }
double FunctionFlat_Tan_evaluate(Node* node) { node->data.functionFlat.coefficient * pow(tan(node->data.functionFlat.input->evaluate(node->data.functionFlat.input)), node->data.functionFlat.exponent); }

float FunctionFlat_Sin_evaluateFloat(Node* node) { node->data.functionFlat.coefficient * powf(sinf(node->data.functionFlat.input->evaluateFloat(node->data.functionFlat.input)), node->data.functionFlat.exponent); }
float FunctionFlat_Cos_evaluateFloat(Node* node) { node->data.functionFlat.coefficient * powf(cosf(node->data.functionFlat.input->evaluateFloat(node->data.functionFlat.input)), node->data.functionFlat.exponent); }
float FunctionFlat_Tan_evaluateFloat(Node* node) { node->data.functionFlat.coefficient * powf(tanf(node->data.functionFlat.input->evaluateFloat(node->data.functionFlat.input)), node->data.functionFlat.exponent); }

Node* FunctionFlat_derivative(Node* node, char var) {
    return NULL;
}
Node* FunctionFlat_antiderivative(Node* node, char var) {
    return NULL;
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
// Operator

Node* Operator(OperationType operation, Node* left, Node* right) {
    Node* node = (Node*)malloc(sizeof(Node));
    if (node) {
        node->type = NODE_OPERATOR;
        node->data.operator.operation = operation;
        node->data.operator.left = left;
        node->data.operator.right = right;
        switch(operation) {
            case OPERATION_ADD:
                node->evaluate = Operator_Add_evaluate;
                node->evaluateFloat = Operator_Add_evaluateFloat;
                break;
            case OPERATION_SUBTRACT:
                node->evaluate = Operator_Subtract_evaluate;
                node->evaluateFloat = Operator_Subtract_evaluateFloat;
                break;
            case OPERATION_MULTIPLY:
                node->evaluate = Operator_Multiply_evaluate;
                node->evaluateFloat = Operator_Multiply_evaluateFloat;
                break;
            case OPERATION_DIVIDE:
                node->evaluate = Operator_Divide_evaluate;
                node->evaluateFloat = Operator_Divide_evaluateFloat;
                break;
            case OPERATION_POWER:
                node->evaluate = Operator_Power_evaluate;
                node->evaluateFloat = Operator_Power_evaluateFloat;
                break;
        }
        node->derivative = Operator_derivative;
        node->antiderivative = Operator_antiderivative;
    }
    return node;
}
double Operator_Add_evaluate(Node* node) { node->data.operator.left->evaluate(node->data.operator.left) + node->data.operator.right->evaluate(node->data.operator.right); }
double Operator_Subtract_evaluate(Node* node) { node->data.operator.left->evaluate(node->data.operator.left) - node->data.operator.right->evaluate(node->data.operator.right); }
double Operator_Multiply_evaluate(Node* node) { node->data.operator.left->evaluate(node->data.operator.left) * node->data.operator.right->evaluate(node->data.operator.right); }
double Operator_Divide_evaluate(Node* node) { node->data.operator.left->evaluate(node->data.operator.left) / node->data.operator.right->evaluate(node->data.operator.right); }
double Operator_Power_evaluate(Node* node) { pow(node->data.operator.left->evaluate(node->data.operator.left), node->data.operator.right->evaluate(node->data.operator.right)); }

float Operator_Add_evaluateFloat(Node* node) { node->data.operator.left->evaluateFloat(node->data.operator.left) + node->data.operator.right->evaluateFloat(node->data.operator.right); }
float Operator_Subtract_evaluateFloat(Node* node) { node->data.operator.left->evaluateFloat(node->data.operator.left) - node->data.operator.right->evaluateFloat(node->data.operator.right); }
float Operator_Multiply_evaluateFloat(Node* node) { node->data.operator.left->evaluateFloat(node->data.operator.left) * node->data.operator.right->evaluateFloat(node->data.operator.right); }
float Operator_Divide_evaluateFloat(Node* node) { node->data.operator.left->evaluateFloat(node->data.operator.left) / node->data.operator.right->evaluateFloat(node->data.operator.right); }
float Operator_Power_evaluateFloat(Node* node) { powf(node->data.operator.left->evaluateFloat(node->data.operator.left), node->data.operator.right->evaluateFloat(node->data.operator.right)); }

Node* Operator_derivative(Node* node, char var) {
    return NULL;
}
Node* Operator_antiderivative(Node* node, char var) {
    return NULL;
}

