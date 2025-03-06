#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>
#include <keypadc.h>
#include <tice.h>

#include "font.h"
#include "input.h"
#include "global.h"
#include "graph/display.h"

#include "mathmatics/parse.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int (*Catalog_Function_Pointer)(void);

typedef struct {
    char name[50];
    Catalog_Function_Pointer function;
} Catalog_Function;

Catalog_Function* catalog[50];
int catalog_size = 0;

void registerFunction(const char* name, Catalog_Function_Pointer function) {
    if (catalog_size < 50) {
        catalog[catalog_size] = (Catalog_Function*) malloc(sizeof(Catalog_Function));
        strncpy(catalog[catalog_size]->name, name, 50);
        catalog[catalog_size]->function = function;
        catalog_size++;
    } else {
        //printf("Catalog is full!\n");
    }
}

Catalog_Function_Pointer getFunction(const char* name) {
    for (int i = 0; i < catalog_size; i++) {
        if (strcmp(catalog[i]->name, name) == 0) {
            return catalog[i]->function;
        }
    }
    return NULL;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {
    gfx_Begin();

    initializeFonts();
    VariableMap* variableMap = createVariableMap(50);
    for(int index = 'a'; index <= 'z'; index++) {
        variableMapPut(variableMap, index, index - 'a'); // just a simple init
    }

    InputHandler* input = createInputHandler();
    if(input) {
        input->window.x = 30;
        input->window.y = 30;
        input->window.width = 250;
        input->window.height = 200;
        // input->scale = 2;
    }

    recordInput(input);

    Node* root = InputHandlerToAbstractSyntaxTree(input);

    if(root) {
        ErrorCode status = SUCCESS;
        float result = evaluateNode(root, variableMap, &status);

        if(!status) {
            gfx_SetTextXY(10, 10);
            gfx_PrintInt(result, 8);
        }
        else {
            gfx_SetTextXY(10, 10);
            gfx_PrintInt(-1, 8);
        }
    } else {
        gfx_SetTextXY(10, 10);
        gfx_PrintInt(-2, 8);
    }

    while(!os_GetCSC());

    gfx_End();

    return 0;
}


