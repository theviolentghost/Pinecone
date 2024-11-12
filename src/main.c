#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include "input.h"
//


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef double (*Catalog_Function_Pointer)(int*);

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
    //float variables[26]= {0};
    //variables['x' - 'a'] = 78; //set x 

    /*Node* root = Operator('/', Constant(5), Variable('x'));
    printf("%f\n", root->evaluate(root, variables));*/

    /*Node* c = Constant(.1285548112);
    printf("%s", Constant_toString(c, true));*/

    /*Node* root = Operator('-', Variable('x'), Constant(5));
    //Node* final = root->derivative(root, 'x');

    printf("%s", Node_toString(root, false));*/

    gfx_Begin();

    initializeFonts();

    Input_handler* input = createInputHandler(50);
    if (input) {
        input->maxBounds->x = 50;
        input->maxBounds->y = 50;
        input->maxBounds->width = 220;
        input->maxBounds->height = 150;
        input->currentBounds->x = 50;
        input->currentBounds->y = 50;
        input->scale = 1;
        recordInput(input, 1);
        freeInputHandler(input);
    }
    gfx_End();

    return 0;
}


