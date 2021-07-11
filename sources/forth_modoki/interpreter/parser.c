#include "clesson.h"
#include "token.h"
#include "parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define NAME_SIZE 256

// character predicates

int is_eof(int c) {
    return c == EOF;
}

int is_numeric(int c) {
    return '0' <= c && c <= '9';
}

int is_space(int c) {
    return c == ' ';
}

int is_open_curly(int c) {
    return c == '{';
}

int is_close_curly(int c) {
    return c == '}';
}

int is_slash(int c) {
    return c == '/';
}

int is_alpha(int c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

int str2int(char *str, int length) {
    int val = 0;
    int exp = 1;

    for (int i = length - 1; i >= 0; i--) {
        val += (str[i] - '0') * exp;
        exp *= 10;
    }
    return val;
}

//// parser

int parse_one(int prev_ch, Token *out_token) {
    int pos = 0;
    int c;
    char buf[NAME_SIZE];

    if (prev_ch == EOF) {
        c = cl_getc();
    } else {
        c = prev_ch;
    }

    if (is_eof(c)) {
        out_token->ltype = END_OF_FILE;
        return EOF;

    } else if (is_open_curly(c)) {
        out_token->ltype = OPEN_CURLY;
        out_token->u.onechar = c;
        return EOF;

    } else if (is_close_curly(c)) {
        out_token->ltype = CLOSE_CURLY;
        out_token->u.onechar = c;
        return EOF;

    } else if (is_numeric(c)) {
        do {
            buf[pos++] = c;
        } while (c = cl_getc(), is_numeric(c));

        out_token->ltype = NUMBER;
        out_token->u.number = str2int(buf, pos);
        return c;

    } else if (is_space(c)) {
        while (c = cl_getc(), is_space(c));
        out_token->ltype = SPACE;
        return c;

    } else if (is_alpha(c)) {
        do {
            buf[pos++] = c;
        } while (c = cl_getc(), (!is_space(c) && !is_eof(c)));
        buf[pos] = '\0';

        out_token->ltype = EXECUTABLE_NAME;
        out_token->u.name = (char *)malloc(sizeof(char) * NAME_SIZE);
        strncpy(out_token->u.name, buf, NAME_SIZE);
        return c;

    } else if (is_slash(c)) {
        while (c = cl_getc(), (!is_space(c) && !is_eof(c))) {
            buf[pos++] = c;
        }
        buf[pos] = '\0';

        out_token->ltype = LITERAL_NAME;
        out_token->u.name = (char *)malloc(sizeof(char) * NAME_SIZE);
        strncpy(out_token->u.name, buf, NAME_SIZE);
        return c;

    }

    out_token->ltype = UNKNOWN;
    return EOF;
}


void parser_print_all() {
    int ch = EOF;
    struct Token token = {
        UNKNOWN,
        {0}
    };

    do {
        ch = parse_one(ch, &token);
        print_token(&token);
    }while(ch != EOF);
}

//// unit tests

static void test_parse_one_number() {
    char *input = "123";
    int expect = 123;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == NUMBER);
    assert(expect == token.u.number);
}

static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    int expect = END_OF_FILE;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect);
}

static void test_parse_one_executable_name() {
    char* input = "add";
    char* expect_name = "add";
    int expect_type = EXECUTABLE_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(strcmp(token.u.name, expect_name) == 0);
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    char* expect_name = "add";
    int expect_type = LITERAL_NAME;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(strcmp(token.u.name, expect_name) == 0);
}

static void test_parse_one_open_curly() {
    char* input = "{";
    char expect_char = '{';
    int expect_type = OPEN_CURLY;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(token.u.onechar == expect_char);
}

static void test_parse_one_close_curly() {
    char* input = "}";
    char expect_char = '}';
    int expect_type = CLOSE_CURLY;

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token.ltype == expect_type);
    assert(token.u.onechar == expect_char);
}

static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();

    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();
}

#ifdef PARSER_TEST
int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    // parser_print_all();
    return 0;
}
#endif
