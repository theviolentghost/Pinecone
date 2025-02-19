#include "./variableMap.h"
#include <stdlib.h>
#include <string.h>

static unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    return hash;
}

VariableMap* createVariableMap(size_t bucketCount) {
    VariableMap* map = malloc(sizeof(VariableMap));
    if (!map) return NULL;
    map->bucketCount = bucketCount;
    map->buckets = calloc(bucketCount, sizeof(VariableMapEntry*));
    if (!map->buckets) {
        free(map);
        return NULL;
    }
    return map;
}

void freeVariableMap(VariableMap* map) {
    if (!map) return;
    for (size_t i = 0; i < map->bucketCount; i++) {
        VariableMapEntry* entry = map->buckets[i];
        while (entry) {
            VariableMapEntry* tmp = entry;
            entry = entry->next;
            free(tmp->key);
            // If needed, free tmp->value here (depending on ownership)
            free(tmp);
        }
    }
    free(map->buckets);
    free(map);
}

int variableMapPut(VariableMap* map, const char* key, Node* value) {
    if (!map || !key) return -1;
    unsigned long idx = hash(key) % map->bucketCount;
    VariableMapEntry* entry = map->buckets[idx];
    
    // Update if key exists.
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            entry->value = value;
            return 0;
        }
        entry = entry->next;
    }
    
    // Insert new entry.
    VariableMapEntry* newEntry = malloc(sizeof(VariableMapEntry));
    if (!newEntry) return -1;
    newEntry->key = strdup(key);
    if (!newEntry->key) {
        free(newEntry);
        return -1;
    }
    newEntry->value = value;
    newEntry->next = map->buckets[idx];
    map->buckets[idx] = newEntry;
    
    return 0;
}

Node* variableMapGet(VariableMap* map, const char* key) {
    if (!map || !key) return NULL;
    unsigned long idx = hash(key) % map->bucketCount;
    VariableMapEntry* entry = map->buckets[idx];
    
    while (entry) {
        if (strcmp(entry->key, key) == 0)
            return entry->value;
        entry = entry->next;
    }
    return NULL;
}