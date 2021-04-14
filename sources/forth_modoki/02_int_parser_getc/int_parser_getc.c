#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/

typedef enum token_type {
    SPACE,
    NUMBER
} token_type;

int parse_one(int prev_c, token_type *out_type, int *out_val) {
    return '\0';
}

int main() {
    int answer1 = 0;
    int answer2 = 0;

    // write something here.

    // sample for cl_getc() usage.
    int c;

    while((c = cl_getc()) != EOF) {
        printf("%c\n",c );
    }

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 0;


}