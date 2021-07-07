#include "clesson.h"
#include "util.h"

#include "token.h"
#include "element.h"

#include "parser.h"
#include "stack.h"
#include "dict.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static Stack *stack;
static Dict *dict;


void add_op() {
    Element a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.etype = ELEMENT_NUMBER;
    result.u.number = a.u.number + b.u.number;

    stack_push(stack, &result);
}

void sub_op() {
    Element a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.etype = ELEMENT_NUMBER;
    result.u.number = b.u.number - a.u.number;

    stack_push(stack, &result);
}

void mul_op() {
    Element a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.etype = ELEMENT_NUMBER;
    result.u.number = b.u.number * a.u.number;

    stack_push(stack, &result);
}

void div_op() {
    Element a, b, result;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    result.etype = ELEMENT_NUMBER;
    result.u.number = (int)(b.u.number / a.u.number);

    stack_push(stack, &result);
}

void def_op() {
    Element name, val;

    stack_pop(stack, &val);
    stack_pop(stack, &name);

    assert(name.etype == ELEMENT_LITERAL_NAME);

    dict_put(dict, name.u.name, &val);
}

void token_to_element(Token *token, Element *e) {
    switch (token->ltype) {
    case NUMBER:
        e->etype = ELEMENT_NUMBER;
        e->u.number = token->u.number;
        break;

    case EXECUTABLE_NAME:
        e->etype = ELEMENT_EXECUTABLE_NAME;
        e->u.name = token->u.name;
        break;

    case LITERAL_NAME:
        e->etype = ELEMENT_LITERAL_NAME;
        e->u.name = token->u.name;
        break;

    default:
        printf("cannot convert Token: ");
        print_token(token);
    }
}

#define MAX_NAME_OP_NUMBERS 256

int compile_exec_array(Element *out_elem) {
    Token token;
    Element elem;
    int ch = EOF;

    Element array[MAX_NAME_OP_NUMBERS];
    int idx = 0;

    do {
        ch = parse_one(ch, &token);

        switch (token.ltype) {
        case NUMBER:
        case EXECUTABLE_NAME:
        case LITERAL_NAME:
            token_to_element(&token, &elem);
            copy_element(&array[idx++], &elem);
            break;

        case OPEN_CURLY:
            break;

        case CLOSE_CURLY:
            // ElementArrayをつくってElementにつめる
            return ch;

        case END_OF_FILE:
            printf("Unexpected EOF\n");
            break;
        }
    } while (ch != EOF);

    return ch;
}

void eval() {
    Token token;
    Element elem;
    int ch = EOF;
    int found;

    do {
        ch = parse_one(ch, &token);
        Element tmp;

        switch (token.ltype) {
        case NUMBER:
            token_to_element(&token, &elem);
            stack_push(stack, &elem);
            break;

        case EXECUTABLE_NAME:
            found = dict_get(dict, token.u.name, &tmp);

            if (found == DICT_FOUND) {
                if (tmp.etype == ELEMENT_C_FUNC) {
                    tmp.u.cfunc();
                } else {
                    // ユーザがdefしたnameなので値をスタックにプッシュ
                    stack_push(stack, &tmp);
                }
            } else {
                // ユーザがevalに渡したnameなのでスタックにプッシュ
                token_to_element(&token, &elem);
                stack_push(stack, &elem);
            }
            break;

        case LITERAL_NAME:
            token_to_element(&token, &elem);
            stack_push(stack, &elem);
            break;

        case OPEN_CURLY:
            ch = compile_exec_array(&elem);
            stack_push(stack, &elem);
            break;

        case SPACE:
            break;

        default:
            printf("unknown token type: %d\n", token.ltype);
        }
    } while (ch != EOF);
}

void register_op(char *name, void (*cfunc)()) {
    Element elem;
    elem.etype = ELEMENT_C_FUNC;
    elem.u.cfunc = cfunc;
    dict_put(dict, name, &elem);
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

    Element elem;
    int stack_ret = stack_pop(stack, &elem);

    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;

    assert(expect == actual);
}

static void test_eval_num_two() {
    char *input = "123 456";
    int expect1 = 456;
    int expect2 = 123;

    eval_with_init(input);

    Element elem1;
    Element elem2;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem1);
    assert(stack_ret == 1);
    assert(elem1.etype == ELEMENT_NUMBER);

    stack_ret = stack_pop(stack, &elem2);
    assert(stack_ret == 0);
    assert(elem2.etype == ELEMENT_NUMBER);

    int actual1 = elem1.u.number;
    int actual2 = elem2.u.number;

    assert(expect1 == actual1);
    assert(expect2 == actual2);
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    int expect = 3;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;
    assert(expect == actual);
}

static void test_eval_complex_add() {
    char *input = "1 2 3 add add 4 5 6 7 8 9 add add add add add add";
    int expect = 45;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;
    assert(expect == actual);
}

static void test_eval_sub() {
    char *input = "5 3 sub";
    int expect = 2;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;
    assert(expect == actual);
}

static void test_eval_mul() {
    char *input = "5 3 mul";
    int expect = 15;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;
    assert(expect == actual);
}

static void test_eval_div() {
    char *input = "7 3 div";
    int expect = 2;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == ELEMENT_NUMBER);

    int actual = elem.u.number;
    assert(expect == actual);
}

static void test_eval_literal_name() {
    char *input = "/foo";
    char *expect_name = "foo";
    char expect_type = ELEMENT_LITERAL_NAME;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expect_type);
    assert(streq(elem.u.name, expect_name));
}

static void test_eval_def() {
    char *input = "/name 42 def name";
    int expect_type = ELEMENT_NUMBER;
    int expect_value = 42;

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expect_type);
    assert(elem.u.number == expect_value);
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

    Element elem;
    stack_pop(stack, &elem);
    print_element(&elem);

    return 0;
}
#endif
