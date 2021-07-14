#include "clesson.h"
#include "util.h"
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

int is_comment(int c) {
    return c == '%';
}

int is_newline(int c) {
    return c == '\n';
}

int is_numeric(int c) {
    return '0' <= c && c <= '9';
}

int is_space(int c) {
    return c == ' ' || is_newline(c);
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

int is_end_of_name(int c) {
    return is_space(c) || is_close_curly(c) || is_open_curly(c) || is_eof(c);
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

    } else if (is_comment(c)) {
        while (c = cl_getc(), !is_newline(c)) ;
        out_token->ltype = COMMENT;
        return c;

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
        } while (c = cl_getc(), !is_end_of_name(c));
        buf[pos] = '\0';

        out_token->ltype = EXECUTABLE_NAME;
        out_token->u.name = (char *)malloc(sizeof(char) * NAME_SIZE);
        strncpy(out_token->u.name, buf, NAME_SIZE);
        return c;

    } else if (is_slash(c)) {
        while (c = cl_getc(), !is_end_of_name(c)) {
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
    Token expected = {NUMBER, {123}};

    Token token;
    int ch;

    cl_getc_set_src(input);

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_empty_should_return_END_OF_FILE() {
    char *input = "";
    Token expected = {END_OF_FILE};

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_executable_name() {
    char* input = "add";
    Token expected = {EXECUTABLE_NAME, {name: "add"}};

    Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_literal_name() {
    char* input = "/add";
    Token expected = {LITERAL_NAME, {name: "add"}};

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_open_curly() {
    char* input = "{";
    Token expected = {OPEN_CURLY, {onechar: '{'}};

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_close_curly() {
    char* input = "}";
    Token expected = {CLOSE_CURLY, {onechar: ']'}};

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_liteal_name_with_close_curly() {
    char *input = "/test}";
    Token expected = {LITERAL_NAME, {name: "test"}};
    char peeked_char = '}';

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == peeked_char);
    assert(token_equal(&token, &expected));
}

static void test_parse_one_executable_name_with_open_curly() {
    char *input = "test{";
    Token expected = {EXECUTABLE_NAME, {name: "test"}};
    char peeked_char = '{';

    Token token;
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == peeked_char);
    assert(token_equal(&token, &expected));
}

static void test_parse_two_names_delimited_by_newline() {
    char *input = "aaa\nbbb";
    Token expected1 = {EXECUTABLE_NAME, {name: "aaa"}};
    Token expected2 = {EXECUTABLE_NAME, {name: "bbb"}};

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == '\n');
    assert(token_equal(&token, &expected1));

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected2));
}

static void test_parse_one_name_with_comment() {
    char *input = "% this is a comment line.\ntest";
    Token expected1 = {COMMENT, {}};
    Token expected2 = {EXECUTABLE_NAME, {name: "test"}};
    char peeked_char = '\n';

    struct Token token = {UNKNOWN, {0}};
    int ch;

    cl_getc_set_src(input);
    ch = parse_one(EOF, &token);

    assert(ch == peeked_char);
    assert(token_equal(&token, &expected1));

    ch = parse_one(EOF, &token);

    assert(ch == EOF);
    assert(token_equal(&token, &expected2));
}


static void unit_tests() {
    test_parse_one_empty_should_return_END_OF_FILE();
    test_parse_one_number();

    test_parse_one_executable_name();
    test_parse_one_literal_name();
    test_parse_one_open_curly();
    test_parse_one_close_curly();

    test_parse_one_liteal_name_with_close_curly();
    test_parse_one_executable_name_with_open_curly();

    test_parse_two_names_delimited_by_newline();
    test_parse_one_name_with_comment();
}

#ifdef PARSER_TEST
int main() {
    unit_tests();

    cl_getc_set_src("123 45 add /some { 2 3 add } def");
    // parser_print_all();
    return 0;
}
#endif
