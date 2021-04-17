#include "clesson.h"
#include "parser.h"
#include "stack.h"

#include <assert.h>

static Stack *stack;

void eval() {}

static void test_eval_num_one() {
    char *input = "123";
    int expect = 123;

    cl_getc_set_src(input);
    stack = stack_initialize();

    eval();

    Token *token;
    int stack_ret = stack_pop(stack, &token);

    assert(stack_ret == 0);
    assert(token->ltype == NUMBER);

    int actual = token->u.number;

    assert(expect == actual);
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    cl_getc_set_src(input);
    stack = stack_initialize();

    eval();

    Token *token1;
    Token *token2;
    int stack_ret;

    stack_ret = stack_pop(stack, &token1);
    assert(stack_ret == 1);
    assert(token1->ltype == NUMBER);

    stack_ret = stack_pop(stack, &token2);
    assert(stack_ret == 1);
    assert(token2->ltype == NUMBER);

    int actual1 = token1->u.number;
    int actual2 = token2->u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    cl_getc_set_src(input);

    eval();

    Token *token;
    int stack_ret;

    stack_ret = stack_pop(stack, &token);
    assert(stack_ret == 1);
    assert(token->ltype == NUMBER);

    int actual = token->u.number;
    assert(expect == actual);
}


int main() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();

    return 0;
}
