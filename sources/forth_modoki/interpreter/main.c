#include "clesson.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>

#define SUCCEEDED 0
#define FAILED 1

#define BUFFER_SIZE 1024 * 10

int cl_getc_set_file(char *filename) {
    FILE *fin = fopen(filename, "r");
    if (fin == NULL) {
        return FAILED;
    }

    char *buf = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    fread(buf, sizeof(char), BUFFER_SIZE, fin);
    cl_getc_set_src(buf);

    fclose(fin);
    return SUCCEEDED;
}

int main(int argc, char **argv) {
    if (argc >= 2) {
        cl_getc_set_file(argv[1]);
    } else {
        cl_getc_set_src("/a 1 def a 2 add");
    }

    initialize_eval();
    eval();
    print_stack();

    return 0;
}
