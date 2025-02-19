#ifndef VARIABLEMAP_H
#define VARIABLEMAP_H

#include <stddef.h>
#include "./AST.h"  // Assuming Node is defined in AST.h

typedef struct VariableMapEntry {
    char *key;
    Node *value; // Usually a Variable node
    struct VariableMapEntry *next;
} VariableMapEntry;

typedef struct {
    VariableMapEntry **buckets;
    size_t bucketCount;
} VariableMap;

// Creates a new variable map with the given number of buckets.
VariableMap* createVariableMap(size_t bucketCount);

// Frees the variable map and all associated memory.
void freeVariableMap(VariableMap* map);

// Inserts or updates an entry into the map.
// Returns 0 on success, non-zero on failure.
int variableMapPut(VariableMap* map, const char* key, Node* value);

// Retrieves the variable associated with the key.
// Returns NULL if the key is not found.
Node* variableMapGet(VariableMap* map, const char* key);

#endif // VARIABLE_MAP_H