#include "clesson.h"
#include "eval.h"
#include <stdio.h>

#define SUCCEEDED 0
#define FAILED 1

int set_file_as_input(char *filename) {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        return FAILED;
    }

    cl_getc_set_file(fin);

    fclose(fin);
    return SUCCEEDED;
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        set_file_as_input(argv[1]);
    } else {
        cl_getc_set_src("/a 1 def a 2 add");
    }

    initialize_eval();
    eval();
    print_stack();

    return 0;
}
