#include "clesson.h"
#include "util.h"

#include "parser.h"
#include "stack.h"
#include "dict.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static Stack *stack;
static Dict *dict;

void eval() {
    Token token;
    int ch = EOF;

    do {
        ch = parse_one(ch, &token);
        Token tmp;

        switch (token.ltype) {
        case NUMBER:
            stack_push(stack, &token);
            break;

        case EXECUTABLE_NAME:
            if (streq(token.u.name, "add")) {
                Token a, b, result;

                stack_pop(stack, &a);
                stack_pop(stack, &b);

                result.ltype = NUMBER;
                result.u.number = a.u.number + b.u.number;

                stack_push(stack, &result);

            } else if (streq(token.u.name, "def")) {
                Token name, val;

                stack_pop(stack, &val);
                stack_pop(stack, &name);

                assert(name.ltype == LITERAL_NAME);

                dict_put(dict, name.u.name, &val);

            } else if (dict_get(dict, token.u.name, &tmp) == DICT_FOUND) {
                stack_push(stack, &tmp);
            }

            break;

        case LITERAL_NAME:
            stack_push(stack, &token);
            break;

        case SPACE:
            break;

        default:
            printf("unknown token type: %d\n", token.ltype);
        }
    } while (ch != EOF);
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();

    eval();

    Token token;
    int stack_ret = stack_pop(stack, &token);

    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;

    assert(expect == actual);
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();

    eval();

    Token token1;
    Token token2;
    int stack_ret;

    stack_ret = stack_pop(stack, &token1);
    assert(stack_ret == 1);
    assert(token1.ltype == NUMBER);

    stack_ret = stack_pop(stack, &token2);
    assert(stack_ret == 0);
    assert(token2.ltype == NUMBER);

    int actual1 = token1.u.number;
    int actual2 = token2.u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();

    eval();

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;
    assert(expect == actual);
}

static void test_eval_complex_add() {
    char *input = "1 2 3 add add 4 5 6 7 8 9 add add add add add add";
    int expect = 45;

    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();

    eval();

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;
    assert(expect == actual);
}

static void test_eval_literal_name() {
    char *input = "/foo";
    char *expect_name = "foo";
    char expect_type = LITERAL_NAME;

    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();

    eval();

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == expect_type);
    assert(streq(token.u.name, expect_name));
}

static void test_all() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_complex_add();

    test_eval_literal_name();
}

int main() {
    //test_all();

    cl_getc_set_src("/abc 42 def abc abc add");
    stack = stack_init();
    dict = dict_init();
    eval();

    Token token;
    stack_pop(stack, &token);
    print_token(&token);

    return 0;
}
