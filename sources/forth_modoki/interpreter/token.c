#include "token.h"

#include <stdio.h>

void token_copy(Token *dest, Token *src) {
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

        default:
            printf("Unknown type %d\n", token->ltype);
            break;
        }
    }
}
