#include "clesson.h"

#include "eval.h"

int main() {
    cl_getc_set_src("/a 1 def a 2 add");

    initialize_eval();
    eval();
    print_stack();

    return 0;
}
