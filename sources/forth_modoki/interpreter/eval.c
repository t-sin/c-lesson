#include "clesson.h"
#include "util.h"

#include "token.h"
#include "element.h"

#include "parser.h"
#include "stack.h"
#include "dict.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
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
    ElementArray *elem_array;
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
            ch = compile_exec_array(&elem);
            copy_element(&array[idx++], &elem);
            break;

        case CLOSE_CURLY:
            elem_array = (struct ElementArray*)malloc(sizeof(ElementArray) + sizeof(Element) * idx);
            elem_array->len = idx;
            memcpy(elem_array->elements, array, sizeof(Element) * idx);

            out_elem->etype = ELEMENT_EXEC_ARRAY;
            out_elem->u.byte_codes = elem_array;
            return EOF;

        case END_OF_FILE:
            printf("Unexpected EOF\n");
            break;
        }
    } while (token.ltype != CLOSE_CURLY);

    return ch;
}

void eval_exec_array(ElementArray *elems) {
    for (int i = 0; i < elems->len; i++) {
        Element elem = elems->elements[i];

        Element tmp;
        int found;

        switch (elem.etype) {
        case ELEMENT_NUMBER:
            stack_push(stack, &elem);
            break;

        case ELEMENT_EXECUTABLE_NAME:
            found = dict_get(dict, elem.u.name, &tmp);

            if (found == DICT_FOUND) {
                if (tmp.etype == ELEMENT_C_FUNC) {
                    tmp.u.cfunc();
                } else if (tmp.etype == ELEMENT_EXEC_ARRAY) {
                    eval_exec_array(tmp.u.byte_codes);
                } else {
                    // ユーザがdefしたnameなので値をスタックにプッシュ
                    stack_push(stack, &tmp);
                }
            } else {
                // これはどういう状況だ…？
                // symbolがunbound的な感じだと思うのでエラーとしたい。
                printf("unbound executable name: %s\n", elem.u.name);
            }
            break;

        case ELEMENT_LITERAL_NAME:
            stack_push(stack, &elem);
            break;

        case ELEMENT_C_FUNC:
            elem.u.cfunc();
            break;

        case ELEMENT_EXEC_ARRAY:
            eval_exec_array(elem.u.byte_codes);
            break;
        }
    }
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
                } else if (tmp.etype == ELEMENT_EXEC_ARRAY) {
                    eval_exec_array(tmp.u.byte_codes);
                } else {
                    // ユーザがdefしたnameなので値をスタックにプッシュ
                    stack_push(stack, &tmp);
                }
            } else {
                // // ユーザがevalに渡したnameなのでスタックにプッシュ
                // token_to_element(&token, &elem);
                // stack_push(stack, &elem);
                // ↑ ちがう気がする。
                // symbolがunbound的な感じだと思うのでエラーとしたい。
                printf("unbound executable name: %s\n", token.u.name);

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

       case CLOSE_CURLY:
            printf("unexpected close curly.\n");
            return;

        case SPACE:
            break;

        case END_OF_FILE:
            break;

        default:
            printf("unknown token type: %d\n", token.ltype);
        }
    } while (token.ltype != END_OF_FILE);
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
    Element expected = {ELEMENT_NUMBER, {123}};

    eval_with_init(input);

    Element elem;
    int stack_ret = stack_pop(stack, &elem);

    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_num_two() {
    char *input = "123 456";
    Element expected1 = {ELEMENT_NUMBER, {456}};
    Element expected2 = {ELEMENT_NUMBER, {123}};

    eval_with_init(input);

    Element elem1;
    Element elem2;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem1);
    assert(stack_ret == 1);
    assert(element_equal(&elem1, &expected1));

    stack_ret = stack_pop(stack, &elem2);
    assert(stack_ret == 0);
    assert(element_equal(&elem2, &expected2));
}


static void test_eval_num_add() {
    char *input = "1 2 add";
    Element expected = {ELEMENT_NUMBER, {3}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_complex_add() {
    char *input = "1 2 3 add add 4 5 6 7 8 9 add add add add add add";
    Element expected = {ELEMENT_NUMBER, {45}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_sub() {
    char *input = "5 3 sub";
    Element expected = {ELEMENT_NUMBER, {2}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_mul() {
    char *input = "5 3 mul";
    Element expected = {ELEMENT_NUMBER, {15}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_div() {
    char *input = "7 3 div";
    Element expected = {ELEMENT_NUMBER, {2}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_literal_name() {
    char *input = "/foo";
    Element expected = {ELEMENT_LITERAL_NAME, {name: "foo"}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_def() {
    char *input = "/name 42 def name";
    Element expected = {ELEMENT_NUMBER, {42}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_exec_array_with_a_number() {
    char *input = "{ 42 }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_NUMBER, {42}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val));
}

static void test_eval_exec_array_with_a_literal_name() {
    char *input = "{ /abc }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_LITERAL_NAME, {name: "abc"}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val));
}

static void test_eval_exec_array_with_a_executable_name() {
    char *input = "{ abc }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_EXECUTABLE_NAME, {name: "abc"}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val));
}

static void test_eval_exec_array_with_two_elements() {
    char *input = "{ 42 43 }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 2;
    Element expected_exec_array_val1 = {ELEMENT_NUMBER, {42}};
    Element expected_exec_array_val2 = {ELEMENT_NUMBER, {43}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val1));
    assert(element_equal(&elem.u.byte_codes->elements[1], &expected_exec_array_val2));
}

static void test_eval_two_exec_array() {
    char *input = "{1} {2}";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array1_val = {ELEMENT_NUMBER, {2}};
    Element expected_exec_array2_val = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 1);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array1_val));

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array2_val));
}

static void test_eval_nested_exec_array() {
    char *input = "{1 {2} 3}";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array1_len = 3;
    int expected_exec_array2_len = 1;
    Element expected_exec_array_val1 = {ELEMENT_NUMBER, {1}};
    Element expected_exec_array2_val = {ELEMENT_NUMBER, {2}};
    Element expected_exec_array_val3 = {ELEMENT_NUMBER, {3}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array1_len);
    assert(element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val1));
    assert(element_equal(&elem.u.byte_codes->elements[2], &expected_exec_array_val3));

    Element *arr = &elem.u.byte_codes->elements[1];
    assert(arr->etype == expected_type);
    assert(arr->u.byte_codes->len = expected_exec_array2_len);
    assert(element_equal(&arr->u.byte_codes->elements[0], &expected_exec_array2_val));
}

static void test_eval_invoke_exec_array_with_a_number() {
    char *input = "/name { 1 } def name";
    Element expected = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem);
    assert(stack_ret == 0);
    assert(element_equal(&elem, &expected));
}

static void test_eval_invoke_nested_exec_array() {
    char *input = "/name1 { 1 } def /name2 { name1 2 } def name2";
    Element expected1 = {ELEMENT_NUMBER, {2}};
    Element expected2 = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem1, elem2;
    int stack_ret;

    stack_ret = stack_pop(stack, &elem1);
    assert(stack_ret == 1);
    assert(element_equal(&elem1, &expected1));

    stack_ret = stack_pop(stack, &elem2);
    assert(stack_ret == 0);
    assert(element_equal(&elem2, &expected2));
}

static void test_eval_invoke_nested_exec_array2() {
    char *input = "/ZZ {6} def /YY {4 ZZ 5} def /XX {1 2 YY 3} def XX";
    int expected_nums[] = {1, 2, 4, 6, 5, 3};

    eval_with_init(input);

    Element elem;
    int stack_ret;

    for (int i = sizeof(expected_nums) / sizeof(expected_nums[0]); i > 0; i--) {
        int idx = i - 1;
        Element expected = {ELEMENT_NUMBER, {expected_nums[idx]}};
        stack_ret = stack_pop(stack, &elem);
        assert(stack_ret == idx);
        assert(element_equal(&elem, &expected));
    }
}

static void test_all() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_num_add();
    test_eval_complex_add();

    test_eval_literal_name();
    test_eval_def();

    test_eval_exec_array_with_a_number();
    test_eval_exec_array_with_a_literal_name();
    test_eval_exec_array_with_a_executable_name();

    test_eval_exec_array_with_two_elements();
    test_eval_two_exec_array();
    test_eval_nested_exec_array();

    test_eval_invoke_exec_array_with_a_number();
    test_eval_invoke_nested_exec_array();
    test_eval_invoke_nested_exec_array2();
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
