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
            printf("<num:%d>", token->u.number);
            break;
        case COMMENT:
            printf("<comment>");
            break;
        case SPACE:
            printf("<space!>");
            break;
        case OPEN_CURLY:
            printf("<open-curly>");
            break;
        case CLOSE_CURLY:
            printf("<close-curly>");
            break;
        case EXECUTABLE_NAME:
            printf("<exec-name:%s>", token->u.name);
            break;
        case LITERAL_NAME:
            printf("<lit_name:%s>", token->u.name);
            break;
        case END_OF_FILE:
            printf("<EOF>");
            break;

        default:
            printf("<Unknown type %d>", token->ltype);
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
