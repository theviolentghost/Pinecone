#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>

typedef int (*Catalog_Function_Pointer)(void);

typedef struct {
    char name[50];
    Catalog_Function_Pointer function;
} Catalog_Function;

extern Catalog_Function* catalog[50];
extern int catalog_size;

void registerFunction(const char* name, Catalog_Function_Pointer function);
//Catalog_Function_Pointer getFunction(const char* name);

#endif
