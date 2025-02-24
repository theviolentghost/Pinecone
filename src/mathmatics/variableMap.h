#ifndef VARIABLEMAP_H
#define VARIABLEMAP_H

#include <stdlib.h>

typedef struct VariableMapEntry {
    char* key;
    float value;
    struct VariableMapEntry* next;
} VariableMapEntry;

typedef struct VariableMap {
    size_t bucketCount;
    VariableMapEntry** buckets;
} VariableMap;

VariableMap* createVariableMap(size_t bucketCount);
void freeVariableMap(VariableMap* map);
int variableMapPut(VariableMap* map, const char* key, float value);
float variableMapGet(VariableMap* map, const char* key);

#endif // VARIABLEMAP_H