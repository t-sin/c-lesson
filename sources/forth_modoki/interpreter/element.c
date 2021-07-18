#include <stdio.h>

#include "util.h"
#include "element.h"

void copy_element(Element *dest, Element *src) {
    dest->etype = src->etype;

    switch (src->etype) {
    case ELEMENT_NUMBER:
    case ELEMENT_PRIMITIVE:
        dest->u.number = src->u.number;
        break;

    case ELEMENT_EXECUTABLE_NAME:
    case ELEMENT_LITERAL_NAME:
        dest->u.name = src->u.name;
        break;

    case ELEMENT_C_FUNC:
        dest->u.cfunc = src->u.cfunc;
        break;

    case ELEMENT_COMPILE_FUNC:
        dest->u.compile_func = src->u.compile_func;
        break;

    case ELEMENT_EXEC_ARRAY:
        dest->u.byte_codes = src->u.byte_codes;
        break;
    }
}

void print_element(Element *e) {
    switch(e->etype) {
    case ELEMENT_NUMBER:
        printf("<num:%d>", e->u.number);
        break;

    case ELEMENT_EXECUTABLE_NAME:
        printf("<exec-name:%s>", e->u.name);
        break;

    case ELEMENT_LITERAL_NAME:
        printf("<lit-name:%s>", e->u.name);
        break;

    case ELEMENT_C_FUNC:
        printf("<cfunc>");
        break;

    case ELEMENT_COMPILE_FUNC:
        printf("<compile_func>");
        break;

    case ELEMENT_PRIMITIVE:
        printf("<inst:%d>", e->u.number);
        break;

    case ELEMENT_EXEC_ARRAY:
        printf("<exec-array:%d,{", e->u.byte_codes->len);
        for (int i = 0; i < e->u.byte_codes->len; i++) {
            printf(" ");
            print_element(&e->u.byte_codes->elements[i]);
        }
        printf("}>");
        break;

    default:
        printf("<unknown:%d>", e->etype);
    }
}

int element_equal(Element *a, Element *b) {
    if (a->etype != b->etype) {
        return 0;
    }

    switch (a->etype) {
    case ELEMENT_NUMBER:
    case ELEMENT_PRIMITIVE:
        return a->u.number == b->u.number;

    case ELEMENT_EXECUTABLE_NAME:
    case ELEMENT_LITERAL_NAME:
        return streq(a->u.name, b->u.name);

    case ELEMENT_C_FUNC:
        return a->u.cfunc == b->u.cfunc;

    case ELEMENT_COMPILE_FUNC:
        return a->u.compile_func == b->u.compile_func;

    case ELEMENT_EXEC_ARRAY:
        if (a->u.byte_codes->len != b->u.byte_codes->len) {
            return 0;
        }

        for (int i = 0; i < a->u.byte_codes->len; i++) {
            Element *e1 = &a->u.byte_codes->elements[i];
            Element *e2 = &b->u.byte_codes->elements[i];
            if (!element_equal(e1, e2)) {
                return 0;
            }
        }
        return 1;
    }
}

int element_is_true(Element *e) {
    if (e->etype == ELEMENT_NUMBER) {
        return e->u.number == 1;
    }
    return 0;
}

int element_is_false(Element *e) {
    if (e->etype == ELEMENT_NUMBER) {
        return e->u.number == 0;
    }
    return 0;
}
