#include "clesson.h"
#include "util.h"

#include "token.h"
#include "parser.h"
#include "stack.h"
#include "dict.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static Stack *stack;
static Dict *dict;

void add_op() {
    Token a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.ltype = NUMBER;
    result.u.number = a.u.number + b.u.number;

    stack_push(stack, &result);
}

void sub_op() {
    Token a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.ltype = NUMBER;
    result.u.number = b.u.number - a.u.number;

    stack_push(stack, &result);
}

void mul_op() {
    Token a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.ltype = NUMBER;
    result.u.number = b.u.number * a.u.number;

    stack_push(stack, &result);
}

void div_op() {
    Token a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.ltype = NUMBER;
    result.u.number = (int)(b.u.number / a.u.number);

    stack_push(stack, &result);
}

void def_op() {
    Token name, val;

    stack_pop(stack, &val);
    stack_pop(stack, &name);

    assert(name.ltype == LITERAL_NAME);

    dict_put(dict, name.u.name, &val);
}

void eval() {
    Token token;
    int ch = EOF;
    int found;

    do {
        ch = parse_one(ch, &token);
        Token tmp;

        switch (token.ltype) {
        case NUMBER:
            stack_push(stack, &token);
            break;

        case EXECUTABLE_NAME:
            found = dict_get(dict, token.u.name, &tmp);

            if (found == DICT_FOUND) {
                if (tmp.ltype == C_FUNC) {
                    tmp.u.cfunc();
                } else {
                    // ユーザがdefしたnameなので値をスタックにプッシュ
                    stack_push(stack, &tmp);
                }
            } else {
                // ユーザがevalに渡したnameなのでスタックにプッシュ
                stack_push(stack, &token);
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

void register_op(char *name, void (*cfunc)()) {
    Token token;
    token.ltype = C_FUNC;
    token.u.cfunc = cfunc;
    dict_put(dict, name, &token);
}

void register_primitives() {
    register_op("add", add_op);
    register_op("sub", sub_op);
    register_op("mul", mul_op);
    register_op("div", div_op);
    register_op("def", def_op);
}

void eval_with_init(char *input) {
    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();
    register_primitives();

    eval();
}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    eval_with_init(input);

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

    eval_with_init(input);

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

    eval_with_init(input);

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

    eval_with_init(input);

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;
    assert(expect == actual);
}

static void test_eval_sub() {
    char *input = "5 3 sub";
    int expect = 2;

    eval_with_init(input);

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;
    assert(expect == actual);
}

static void test_eval_mul() {
    char *input = "5 3 mul";
    int expect = 15;

    eval_with_init(input);

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == NUMBER);

    int actual = token.u.number;
    assert(expect == actual);
}

static void test_eval_div() {
    char *input = "7 3 mul";
    int expect = 2;

    eval_with_init(input);

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

    eval_with_init(input);

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == expect_type);
    assert(streq(token.u.name, expect_name));
}

static void test_eval_def() {
    char *input = "/name 42 def name";
    int expect_type = NUMBER;
    int expect_value = 42;

    eval_with_init(input);

    Token token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 0);
    assert(token.ltype == expect_type);
    assert(token.u.number == expect_value);
}

static void test_all() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_complex_add();

    test_eval_literal_name();
    test_eval_def();
}

#ifdef EVAL_TEST
int main() {
    test_all();
    return 0;
}
#else
int main() {
    dict = dict_init();
    register_primitives();

    cl_getc_set_src("/abc 42 def abc abc add");
    stack = stack_init();
    eval();

    Token token;
    stack_pop(stack, &token);
    print_token(&token);

    return 0;
}
#endif