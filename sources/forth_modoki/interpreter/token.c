#include "util.h"
#include "token.h"

#include <stdio.h>

void copy_token(Token *dest, Token *src) {
    dest->ltype = src->ltype;

    switch (src->ltype) {
    case NUMBER:
        dest->u.number = src->u.number;
        break;

    case EXECUTABLE_NAME:
    case LITERAL_NAME:
        dest->u.name = src->u.name;
        break;

    default:
        dest->u.onechar = src->u.onechar;
        break;
    }
}

void print_token(Token *token) {
    if(token->ltype != UNKNOWN) {
        switch(token->ltype) {
        case NUMBER:
            printf("num: %d\n", token->u.number);
            break;
        case COMMENT:
            printf("comment\n");
            break;
        case SPACE:
            printf("space!\n");
            break;
        case OPEN_CURLY:
            printf("Open curly brace '%c'\n", token->u.onechar);
            break;
        case CLOSE_CURLY:
            printf("Close curly brace '%c'\n", token->u.onechar);
            break;
        case EXECUTABLE_NAME:
            printf("EXECUTABLE_NAME: %s\n", token->u.name);
            break;
        case LITERAL_NAME:
            printf("LITERAL_NAME: %s\n", token->u.name);
            break;
        case END_OF_FILE:
            printf("End of file\n");
            break;

        default:
            printf("Unknown type %d\n", token->ltype);
            break;
        }
    }
}

int token_equal(Token *a, Token *b) {
    if (a->ltype != b->ltype) {
        return 0;
    }

    switch (a->ltype) {
    case SPACE:
    case END_OF_FILE:
    case COMMENT:
    case OPEN_CURLY:
    case CLOSE_CURLY:
        return 1;

    case NUMBER:
        return a->u.number == b->u.number;

    case EXECUTABLE_NAME:
    case LITERAL_NAME:
        return streq(a->u.name, b->u.name);

    case UNKNOWN:
        printf("compares two unknown tokens!\n");
        return 1;
    }
}
