#include <stdbool.h>
#include <stdlib.h>

typedef enum {
    NULL_NODE,
    BINARY_NODE,
    CONSTANT_NODE,
    VARIABLE_NODE,
    FUNCTION_NODE,
} NodeType;

struct Node {
    NodeType type;
    union {

    } data;
};