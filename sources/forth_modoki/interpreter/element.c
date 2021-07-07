#include <stdio.h>

#include "element.h"

void copy_element(Element *dest, Element *src) {
    dest->etype = src->etype;

    switch (src->etype) {
    case ELEMENT_NUMBER:
        dest->u.number = src->u.number;
        break;

    case ELEMENT_EXECUTABLE_NAME:
    case ELEMENT_LITERAL_NAME:
        dest->u.name = src->u.name;
        break;

    case ELEMENT_C_FUNC:
        dest->u.cfunc = src->u.cfunc;
        break;
    }
}

void print_element(Element *e) {
    switch(e->etype) {
    case ELEMENT_NUMBER:
        printf("number: %d\n", e->u.number);
        break;

    case ELEMENT_EXECUTABLE_NAME:
        printf("executable name: %s\n", e->u.name);
        break;

    case ELEMENT_LITERAL_NAME:
        printf("literal name: %s\n", e->u.name);
        break;

    case ELEMENT_C_FUNC:
        printf("C function: *\n");
        break;
    }
}
