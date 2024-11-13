#include "../main.h"

// still needs more development before applicable

double my_new_function(double x, double y);

// Initial call when your function is pressed in catalog
// Status codes: 0 (Good)
int run(void) {
    my_new_function(2, 4);

    return 0;
}

//
//
// Start coding your functionality below!

double my_new_function(double x, double y) {
    return x + y;  // Example operation: addition
}

//
//
// Register the new function in catalog object automatically using a constructor
__attribute__((constructor)) void functionRegistration () {
    registerFunction("your_function_name_in_catalog", run);
    //add more instances if applicable
}