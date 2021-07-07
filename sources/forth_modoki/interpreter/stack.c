#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "element.h"
#include "stack.h"

#define STACK_SIZE 1024

struct Stack {
    Element array[STACK_SIZE];
    int top;
};


Stack* stack_init() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->top = 0;

    return stack;
}

int stack_push(Stack *stack, Element *e) {
    if (stack->top < STACK_SIZE) {

        stack->array[stack->top].etype = e->etype;
        stack->array[stack->top].u = e->u;
        stack->top++;
        return stack->top;
    }

    return STACK_FULL;
}

int stack_pop(Stack *stack, Element *out_elem) {
    if (stack->top == 0) {
        return STACK_EMPTY;
    }

    stack->top--;
    out_elem->etype = stack->array[stack->top].etype;
    out_elem->u = stack->array[stack->top].u;

    return stack->top;
}

void stack_print_all(Stack *stack) {
    printf("*****\n");
    for (int i = 0; i < stack->top; i++) {
        printf("  ");
        print_element(&stack->array[i]);
    }
}

void test_initialize_stack() {
    int expected_top = 0;

    Stack *stack = stack_init();

    assert(stack->top == expected_top);
}

void test_pop_empty_stack() {
    int expected_return = STACK_EMPTY;
    int expected_top = 0;
    int actual_return;
    Element e;

    Stack *stack = stack_init();
    actual_return = stack_pop(stack, &e);

    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
}

void test_push_one_integer() {
    int expected_return = 1;
    int expected_top = 1;
    int expected_etype = ELEMENT_NUMBER;
    int expected_value = 42;
    int actual_return;

    Element e = {ELEMENT_NUMBER, {42}};
    Stack *stack = stack_init();

    actual_return = stack_push(stack, &e);

    // 返り値とトップの確認
    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
    // 値の確認
    assert(stack->array[0].etype == expected_etype);
    assert(stack->array[0].u.number == expected_value);
}

void test_push_one_literal_name() {
    Element input = {ELEMENT_LITERAL_NAME, { .name =  "foo" }};
    int expected_top = 1;
    int expected_etype = ELEMENT_LITERAL_NAME;
    char *expected_name = "foo";

    Stack *stack = stack_init();
    int actual_top = stack_push(stack, &input);

    assert(stack->top == expected_top);
    assert(stack->array[0].etype == expected_etype);
    assert(strcmp(stack->array[0].u.name, expected_name) == 0);
}

void test_pop_one_integer() {
    Element input1 = {ELEMENT_NUMBER, {42}};
    Element input2 = {ELEMENT_NUMBER, {420}};

    int expected_etype = ELEMENT_NUMBER;
    int expected_value = 42;

    int expected_return_push = 1;
    int expected_top_push = 1;

    int expected_return_pop = 0;
    int expected_top_pop = 0;

    int actual_return;

    Element out_elem;
    Stack *stack = stack_init();

    actual_return = stack_push(stack, &input1);

    // 返り値とトップの確認
    assert(actual_return == expected_return_push);
    assert(stack->top == expected_top_push);

    actual_return = stack_pop(stack, &out_elem);

    // 返り値とトップの確認
    assert(actual_return == expected_return_pop);
    assert(stack->top == expected_top_pop);
    // 値の確認
    assert(out_elem.etype == expected_etype);
    assert(out_elem.u.number == expected_value);
}

void test_push_to_full_stack() {
    int expected_return = STACK_FULL;
    int expected_top = STACK_SIZE;
    int actual_return;

    Stack *stack = stack_init();
    Element e = {ELEMENT_NUMBER, {42}};

    for (int i = 0; i <= STACK_SIZE - 1; i++) {
        actual_return = stack_push(stack, &e);
    }
    assert(actual_return == STACK_SIZE);
    assert(stack->top == STACK_SIZE);

    // スタックがすべて埋まった状態で要素をプッシュ
    actual_return = stack_push(stack, &e);
    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
}

void test_all() {
    test_pop_empty_stack();
    test_push_one_integer();
    test_push_one_literal_name();
    test_pop_one_integer();
    test_push_to_full_stack();
}

#ifdef STACK_TEST
int main() {
    test_all();

    return 0;
}
#endif