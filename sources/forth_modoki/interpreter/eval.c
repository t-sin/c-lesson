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

//// utilities

void token_to_element(Token *token, Element *out_elem) {
    switch (token->ltype) {
    case NUMBER:
        out_elem->etype = ELEMENT_NUMBER;
        out_elem->u.number = token->u.number;
        break;

    case EXECUTABLE_NAME:
        out_elem->etype = ELEMENT_EXECUTABLE_NAME;
        out_elem->u.name = token->u.name;
        break;

    case LITERAL_NAME:
        out_elem->etype = ELEMENT_LITERAL_NAME;
        out_elem->u.name = token->u.name;
        break;

    default:
        printf("cannot convert Token: ");
        print_token(token);
    }
}

//// byte code compiler

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

//// byte code interpreter

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
            // この処理、compile_exec_arrayでやってもよいのでは？
            // というかForthはそうしてるはず…。
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
            stack_push(stack, &elem);
            break;
        }
    }
}

//// interpreter

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

//// built-in operators

#define define_bin_op(fnname, exp) \
  void fnname() { \
    Element a, b, result; \
    \
    stack_pop(stack, &a); \
    stack_pop(stack, &b); \
    \
    result.etype = ELEMENT_NUMBER; \
    result.u.number = (exp); \
    \
    stack_push(stack, &result); \
  }

define_bin_op(add_op, b.u.number + a.u.number);
define_bin_op(sub_op, b.u.number - a.u.number);
define_bin_op(mul_op, b.u.number * a.u.number);
define_bin_op(div_op, (int)(b.u.number / a.u.number));

define_bin_op(eq_op, element_equal(&a, &b));
define_bin_op(neq_op, !element_equal(&a, &b));

define_bin_op(gt_op, b.u.number > a.u.number);
define_bin_op(ge_op, b.u.number >= a.u.number);
define_bin_op(lt_op, b.u.number < a.u.number);
define_bin_op(le_op, b.u.number <= a.u.number);

void def_op() {
    Element name, val;

    stack_pop(stack, &val);
    stack_pop(stack, &name);

    assert(name.etype == ELEMENT_LITERAL_NAME);

    dict_put(dict, name.u.name, &val);
}

void pstack_op() {
    stack_print_all(stack);
}

void pop_op() {
    Element val;
    stack_pop(stack, &val);
}

void exch_op() {
    Element a, b;

    stack_pop(stack, &a);
    stack_pop(stack, &b);

    stack_push(stack, &a);
    stack_push(stack, &b);
}

void dup_op() {
    Element e;

    stack_pop(stack, &e);

    stack_push(stack, &e);
    stack_push(stack, &e);
}

void index_op() {
    Element n;
    stack_pop(stack, &n);

    Element out;
    stack_peek(stack, n.u.number, &out);

    stack_push(stack, &out);
}

void roll_op() {
    Element n, j;
    stack_pop(stack, &j);
    stack_pop(stack, &n);

    int length = n.u.number;
    int rotn = j.u.number;
    rotn = rotn % length;

    int source_idx = length - 1;
    int dest_idx = source_idx - rotn;

    // 前のコピー先を取っておいて、それを次の位置に書き込む。
    // roll的にはrotnが大きくなるほど右に回転する (正方向が右) が
    // stack_peek等のnはスタックトップから深さ方向にいく (正方向が左) ので
    // 位置計算がじゃっかんややこしいことになっている。注意。
    Element prev_dest;
    stack_peek(stack, source_idx, &prev_dest);

    for (int i = 0; i < length; i++) {
        //printf("%d -> %d, ", source_idx, dest_idx);

        Element tmp;
        copy_element(&tmp, &prev_dest);
        stack_peek(stack, dest_idx, &prev_dest);
        stack_set(stack, dest_idx, &tmp);

        source_idx = dest_idx;
        dest_idx = source_idx - rotn % length;
        if (dest_idx < 0) {
            dest_idx = length + dest_idx;
        }
    }
}

void exec_op() {
    Element e;
    stack_pop(stack, &e);

    eval_exec_array(e.u.byte_codes);
}

//// initilizing codes

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

    register_op("eq", eq_op);
    register_op("neq", neq_op);

    register_op("gt", gt_op);
    register_op("ge", ge_op);
    register_op("lt", lt_op);
    register_op("le", le_op);

    register_op("pstack", pstack_op);

    register_op("pop", pop_op);
    register_op("exch", exch_op);
    register_op("dup", dup_op);
    register_op("index", index_op);
    register_op("roll", roll_op);

    register_op("exec", exec_op);
}

void eval_with_init(char *input) {
    cl_getc_set_src(input);
    stack = stack_init();
    dict = dict_init();
    register_primitives();

    eval();
}

void initialize_eval() {
    stack = stack_init();
    dict = dict_init();
    register_primitives();
}

void print_stack() {
    stack_print_all(stack);
}

//// unit tests

// basic evaluation

static void assert_element_equal(Element *actual, Element *expected) {
    assert(element_equal(actual, expected));
}

static void test_eval_num_one() {
    char *input = "123";
    Element expected = {ELEMENT_NUMBER, {123}};

    eval_with_init(input);

    Element elem;
    int stack_ret = stack_pop(stack, &elem);

    assert(stack_is_empty(stack));
    assert_element_equal(&elem, &expected);
}

static void test_eval_num_two() {
    char *input = "123 456";
    Element expected1 = {ELEMENT_NUMBER, {456}};
    Element expected2 = {ELEMENT_NUMBER, {123}};

    eval_with_init(input);

    Element elem1, elem2;

    stack_pop(stack, &elem1);
    assert_element_equal(&elem1, &expected1);

    stack_pop(stack, &elem2);
    assert_element_equal(&elem2, &expected2);

    assert(stack_is_empty(stack));
}

static void test_eval_literal_name() {
    char *input = "/foo";
    Element expected = {ELEMENT_LITERAL_NAME, {name: "foo"}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert_element_equal(&elem, &expected);

    assert(stack_is_empty(stack));
}

// definiing operator

static void test_eval_def() {
    char *input = "/name 42 def name";
    Element expected = {ELEMENT_NUMBER, {42}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert_element_equal(&elem, &expected);

    assert(stack_is_empty(stack));
}

static void verify_binary_numeral_op(char *input, int expected) {
    Element expected_elem = {ELEMENT_NUMBER, {expected}};

    eval_with_init(input);

    Element elem;
    stack_pop(stack, &elem);
    assert_element_equal(&elem, &expected_elem);

    assert(stack_is_empty(stack));
}

// binary numeral operator

static void test_eval_num_add() {
    char *input = "1 2 add";
    int expected = 3;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_complex_add() {
    char *input = "1 2 3 add add 4 5 6 7 8 9 add add add add add add";
    int expected = 45;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_sub() {
    char *input = "5 3 sub";
    int expected = 2;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_mul() {
    char *input = "5 3 mul";
    int expected = 15;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_div() {
    char *input = "7 3 div";
    int expected = 2;

    verify_binary_numeral_op(input, expected);
}

// numeral predicates

#define TRUE 1
#define FALSE 0

static void test_eval_eq_returns_true() {
    char *input = "42 42 eq";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_eq_returns_false() {
    char *input = "42 43 eq";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_neq_returns_true() {
    char *input = "42 43 neq";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_neq_returns_false() {
    char *input = "42 42 neq";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_gt_returns_true() {
    char *input = "43 42 gt";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_gt_returns_false() {
    char *input = "42 42 gt";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_ge_returns_true() {
    char *input = "42 42 ge";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_ge_returns_false() {
    char *input = "41 42 ge";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_lt_returns_true() {
    char *input = "42 43 lt";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_lt_returns_false() {
    char *input = "42 42 lt";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_le_returns_true() {
    char *input = "42 42 le";
    int expected = TRUE;

    verify_binary_numeral_op(input, expected);
}

static void test_eval_le_returns_false() {
    char *input = "42 41 le";
    int expected = FALSE;

    verify_binary_numeral_op(input, expected);
}

// stack operators

static void assert_stack_integer_contents(int *expected_contents, int expected_length) {
    for (int i = 0; i < expected_length; i++) {
        Element elem;
        stack_pop(stack, &elem);

        assert(elem.etype == ELEMENT_NUMBER);
        assert(elem.u.number == expected_contents[expected_length - i - 1]);
    }

    assert(stack_is_empty(stack));
}

static void test_eval_pop() {
    char *input = "1 2 pop";
    int expected_stack[] = {1};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_exch1() {
    char *input = "1 10 exch";
    int expected_stack[] = {10, 1};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_exch2() {
    char *input = "100 1 10 exch";
    int expected_stack[] = {100, 10, 1};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_dup() {
    char *input = "42 dup";
    int expected_stack[] = {42, 42};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_index1() {
    char *input = "10 20 30 40 0 index";
    int expected_stack[] = {10, 20, 30, 40, 40};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_index2() {
    char *input = "10 20 30 40 2 index";
    int expected_stack[] = {10, 20, 30, 40, 20};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_roll_with_no_rolling() {
    char *input = "1 2 3 3 0 roll";
    int expected_stack[] = {1, 2, 3};
    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);

    eval_with_init(input);

    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_roll_once() {
    char *input = "1 2 3 3 1 roll";
    int expected_stack[] = {3, 1, 2};
    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);

    eval_with_init(input);

    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_roll_twice() {
    char *input = "1 2 3 3 2 roll";
    int expected_stack[] = {2, 3, 1};
    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);

    eval_with_init(input);

    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_roll_three_times() {
    char *input = "1 2 3 3 3 roll";
    int expected_stack[] = {1, 2, 3};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_roll_partially() {
    char *input = "1 2 3 4 5 4 3 roll";
    int expected_stack[] = {1, 3, 4, 5, 2};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_exec() {
    char *input = "{1 2} exec";
    int expected_stack[] = {1, 2};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_eval_exec_nested() {
    char *input = "{1 {2 3} exec} exec";
    int expected_stack[] = {1, 2, 3};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

// executable arrays

static void test_eval_exec_array_with_a_number() {
    char *input = "{ 42 }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_NUMBER, {42}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val);

    assert(stack_is_empty(stack));
}

static void test_eval_exec_array_with_a_literal_name() {
    char *input = "{ /abc }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_LITERAL_NAME, {name: "abc"}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val);

    assert(stack_is_empty(stack));
}

static void test_eval_exec_array_with_a_executable_name() {
    char *input = "{ abc }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array_val = {ELEMENT_EXECUTABLE_NAME, {name: "abc"}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val);

    assert(stack_is_empty(stack));
}

static void test_eval_exec_array_with_two_elements() {
    char *input = "{ 42 43 }";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 2;
    Element expected_exec_array_val1 = {ELEMENT_NUMBER, {42}};
    Element expected_exec_array_val2 = {ELEMENT_NUMBER, {43}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val1);
    assert_element_equal(&elem.u.byte_codes->elements[1], &expected_exec_array_val2);

    assert(stack_is_empty(stack));
}

static void test_eval_two_exec_array() {
    char *input = "{1} {2}";
    int expected_type = ELEMENT_EXEC_ARRAY;
    int expected_exec_array_len = 1;
    Element expected_exec_array1_val = {ELEMENT_NUMBER, {2}};
    Element expected_exec_array2_val = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array1_val);

    stack_pop(stack, &elem);
    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array2_val);

    assert(stack_is_empty(stack));
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

    stack_pop(stack, &elem);
    assert(stack_is_empty(stack));

    assert(elem.etype == expected_type);
    assert(elem.u.byte_codes->len = expected_exec_array1_len);
    assert_element_equal(&elem.u.byte_codes->elements[0], &expected_exec_array_val1);
    assert_element_equal(&elem.u.byte_codes->elements[2], &expected_exec_array_val3);

    Element *arr = &elem.u.byte_codes->elements[1];
    assert(arr->etype == expected_type);
    assert(arr->u.byte_codes->len = expected_exec_array2_len);
    assert_element_equal(&arr->u.byte_codes->elements[0], &expected_exec_array2_val);
}

static void test_eval_invoke_exec_array_with_a_number() {
    char *input = "/name { 1 } def name";
    Element expected = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem;

    stack_pop(stack, &elem);
    assert_element_equal(&elem, &expected);

    assert(stack_is_empty(stack));
}

static void test_eval_invoke_nested_exec_array() {
    char *input = "/name1 { 1 } def /name2 { name1 2 } def name2";
    Element expected1 = {ELEMENT_NUMBER, {2}};
    Element expected2 = {ELEMENT_NUMBER, {1}};

    eval_with_init(input);

    Element elem1, elem2;

    stack_pop(stack, &elem1);
    assert_element_equal(&elem1, &expected1);

    stack_pop(stack, &elem2);
    assert_element_equal(&elem2, &expected2);

    assert(stack_is_empty(stack));
}

static void test_eval_invoke_nested_exec_array2() {
    char *input = "/ZZ {6} def /YY {4 ZZ 5} def /XX {1 2 YY 3} def XX";
    int expected_stack[] = {1, 2, 4, 6, 5, 3};

    eval_with_init(input);

    int expected_length = sizeof(expected_stack) / sizeof(expected_stack[0]);
    assert_stack_integer_contents(expected_stack, expected_length);
}

static void test_all() {
    test_eval_num_one();
    test_eval_num_two();
    test_eval_literal_name();
    test_eval_def();

    test_eval_num_add();
    test_eval_complex_add();
    test_eval_sub();
    test_eval_mul();
    test_eval_div();

    test_eval_eq_returns_true();
    test_eval_eq_returns_false();
    test_eval_neq_returns_true();
    test_eval_neq_returns_false();

    test_eval_gt_returns_true();
    test_eval_gt_returns_false();
    test_eval_ge_returns_true();
    test_eval_ge_returns_false();
    test_eval_lt_returns_true();
    test_eval_lt_returns_false();
    test_eval_le_returns_true();
    test_eval_le_returns_false();

    test_eval_pop();
    test_eval_exch1();
    test_eval_exch2();
    test_eval_dup();
    test_eval_index1();
    test_eval_index2();

    test_eval_roll_with_no_rolling();
    test_eval_roll_once();
    test_eval_roll_twice();
    test_eval_roll_three_times();
    test_eval_roll_partially();

    test_eval_exec();
    test_eval_exec_nested();

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
#endif
