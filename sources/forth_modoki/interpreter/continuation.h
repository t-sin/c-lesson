#ifndef forth_modoki_continuation
#define forth_modoki_continuation

#include "element.h"

#define CONT_FULL -1
#define CONT_EMPTY -2

typedef struct Continuation {
    ElementArray *exec_array;
    int pc;
} Continuation;

void co_reset();
int co_length();
int co_push(Continuation *cont);
int co_pop(Continuation *out_cont);

#endif
