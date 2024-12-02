#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../input.h"
#include "../global.h"

#define HASH_TABLE_SIZE 256 

typedef struct Node Node;
typedef struct VariableLookupNode VariableLookupNode;
typedef struct VariableEntryNode VariableEntryNode;
typedef struct ConstantNode ConstantNode;
typedef struct VariableNode VariableNode;
typedef struct VariableFlatNode VariableFlatNode;
typedef struct FunctionNode FunctionNode;
typedef struct FunctionFlatNode FunctionFlatNode;
typedef struct OperatorNode OperatorNode;

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
    OPERATION_SUBTRACT,
    OPERATION_MULTIPLY,
    OPERATION_DIVIDE,
    OPERATION_POWER,
    OPERATION_EQUAL,
} OperationType;

struct ConstantNode {
    double value;
};
struct VariableNode {
    char name;
    // add derivative support
    Node* coefficient;
    Node* exponent;
};
struct VariableFlatNode {
    char name;
    double coefficient;
    double exponent;
};
struct FunctionNode {
    FunctionName name;
    Node* input;
    Node* base;
    Node* coefficient;
    Node* exponent;
};
struct FunctionFlatNode {
    FunctionName name;
    Node* input;
    float base;
    double coefficient;
    double exponent;
};
struct OperatorNode {
    OperationType operation;
    Node* left;
    Node* right;
};

typedef struct VariableEntry VariableEntry;
typedef struct VariableLookup VariableLookup;

struct VariableEntry {
    char* name;
    double value;
    VariableEntry* next;
};

struct VariableLookup {
    VariableEntry* buckets[HASH_TABLE_SIZE];
    size_t size;
};

VariableLookup* createVariableLookup();
void destroyVariableLookup(VariableLookup* lookup);
VariableEntry* findVariableEntry(VariableLookup* lookup, const char* name);
void insertVariableEntry(VariableLookup* lookup, const char* name, double value);
size_t hash(const char* str);

bool containsVariable(Node* node, char* variable);

Node* Constant(double value);
double Constant_evaluate(Node* node);
float Constant_evaluateFloat(Node* node);
Node* Constant_derivative(Node* node, char var);
Node* Constant_antiderivative(Node* node, char var);

Node* Variable(char name);
double Variable_evaluate(Node* node);
float Variable_evaluateFloat(Node* node);
Node* Variable_derivative(Node* node, char var);
Node* Variable_antiderivative(Node* node, char var);

Node* VariableFlat(char name);
double VariableFlat_evaluate(Node* node);
float VariableFlat_evaluateFloat(Node* node);
Node* VariableFlat_derivative(Node* node, char var);
Node* VariableFlat_antiderivative(Node* node, char var);

Node* Function(FunctionName name, Node* input, Node* base);
double Function_Sin_evaluate(Node* node);
double Function_Cos_evaluate(Node* node);
double Function_Tan_evaluate(Node* node);
float Function_Sin_evaluateFloat(Node* node);
float Function_Cos_evaluateFloat(Node* node);
float Function_Tan_evaluateFloat(Node* node);
Node* Function_derivative(Node* node, char var);
Node* Function_antiderivative(Node* node, char var);

Node* FunctionFlat(FunctionName name, Node* input, float base);
double FunctionFlat_Sin_evaluate(Node* node);
double FunctionFlat_Cos_evaluate(Node* node);
double FunctionFlat_Tan_evaluate(Node* node);
float FunctionFlat_Sin_evaluateFloat(Node* node);
float FunctionFlat_Cos_evaluateFloat(Node* node);
float FunctionFlat_Tan_evaluateFloat(Node* node);
Node* FunctionFlat_derivative(Node* node, char var);
Node* FunctionFlat_antiderivative(Node* node, char var);

Node* Operator(OperationType operation, Node* left, Node* right);
double Operator_Add_evaluate(Node* node);
double Operator_Subtract_evaluate(Node* node);
double Operator_Multiply_evaluate(Node* node);
double Operator_Divide_evaluate(Node* node);
double Operator_Power_evaluate(Node* node);
float Operator_Add_evaluateFloat(Node* node);
float Operator_Subtract_evaluateFloat(Node* node);
float Operator_Multiply_evaluateFloat(Node* node);
float Operator_Divide_evaluateFloat(Node* node);
float Operator_Power_evaluateFloat(Node* node);
Node* Operator_derivative(Node* node, char var);
Node* Operator_antiderivative(Node* node, char var);

#endif