#ifndef forth_modoki_continuation
#define forth_modoki_continuation

#include "element.h"

#define CONT_FULL -1
#define CONT_EMPTY -2

typedef enum ContType {
    CONT_CONT,
    CONT_ELEMENT,
} ContType;

typedef struct Continuation {
    ContType ctype;
    union {
        struct {
            ElementArray *exec_array;
            int pc;
        } c;
        Element e;
    } u;
} Continuation;

void copy_continuation(Continuation *dest, Continuation *src);
void cont_proceed(Continuation *cont, int n);

void co_reset();
int co_length();
int co_push(Continuation *cont);
int co_pop(Continuation *out_cont);

#endif
