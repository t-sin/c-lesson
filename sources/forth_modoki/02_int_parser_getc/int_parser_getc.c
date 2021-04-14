#include "clesson.h"
#include <assert.h>

/*
cc cl_getc.c int_parser_getc.c
*/

typedef enum token_type {
    TOKEN_SPACE,
    TOKEN_NUMBER
} token_type;

int is_numeric(int c) {
    return '0' <= c && c <= '9';
}

int is_space(int c) {
    return c == ' ';
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

int parse_one(int prev_c, token_type *out_type, int *out_val) {
    int pos = 0;
    int c;
    char buf[256];

    if (prev_c == '\0') {
        c = cl_getc();
    } else {
        c = prev_c;
    }

    if (is_numeric(c)) {
        do {
            buf[pos++] = c;
        } while (c = cl_getc(), is_numeric(c));
        *out_type = TOKEN_NUMBER;
        buf[pos] = '\0';
        *out_val = str2int(buf, pos);

    } else if (is_space(c)) {
        while (c = cl_getc(), is_space(c));
        *out_type = TOKEN_SPACE;

    } else {
        // printf("unkwon character: '%c'\n", c);
    }

    return c;
}

void test_parse_one_123() {
    token_type token_type;
    int val;
    int prev_c = '\0';

    cl_getc_set_src("123");
    prev_c = parse_one(prev_c, &token_type, &val);

    assert(prev_c == EOF);
    assert(token_type == TOKEN_NUMBER);
    assert(val == 123);
}

void test_parse_one_123_456() {
    token_type token_type;
    int val;
    int prev_c = '\0';

    cl_getc_set_src("123 456");
    prev_c = parse_one(prev_c, &token_type, &val);
    assert(prev_c == ' ');
    assert(token_type == TOKEN_NUMBER);
    assert(val == 123);

    prev_c = parse_one(prev_c, &token_type, &val);
    assert(prev_c == '4');
    assert(token_type == TOKEN_SPACE);

    prev_c = parse_one(prev_c, &token_type, &val);
    assert(prev_c == EOF);
    assert(token_type == TOKEN_NUMBER);
    assert(val == 456);
}

int main() {
    int answer1 = 0;
    int answer2 = 0;

    test_parse_one_123();
    // 入力を元に戻す
    cl_getc_set_src("123 456");

    token_type token_type;
    int val;
    int prev_c = '\0';

    prev_c = parse_one(prev_c, &token_type, &val);
    answer1 = val;
    prev_c = parse_one(prev_c, &token_type, &val);
    prev_c = parse_one(prev_c, &token_type, &val);
    answer2 = val;

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);

    return 0;
}