#include <assert.h>
#include <stdlib.h>
#include "stack.h"

#define STACK_SIZE 1024

Stack* stack_initialize() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->array = (Token **)malloc(sizeof(Token *) * STACK_SIZE);
    stack->top = 0;

    return stack;
}

int stack_push(Stack *stack, Token *token) {
    if (stack->top < STACK_SIZE) {
        stack->array[stack->top] = token;
        stack->top++;
        return stack->top;
    }

    return STACK_FULL;
}

int stack_pop(Stack *stack, Token *out_token) {
    if (stack->top == 0) {
        return STACK_EMPTY;
    }

    return 0;
}

void test_pop_empty_stack() {
    int expected_return = STACK_EMPTY;
    int expected_top = 0;
    int actual_return;
    Token token;

    Stack *stack = stack_initialize();
    actual_return = stack_pop(stack, &token);

    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
}

void test_push_one_integer() {
    int expected_return = 1;
    int expected_top = 1;
    int expected_ltype = NUMBER;
    int expected_value = 42;
    int actual_return;

    Token token = {NUMBER, {42}};
    Stack *stack = stack_initialize();

    actual_return = stack_push(stack, &token);

    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
    assert(stack->array[0]->ltype == expected_ltype);
    assert(stack->array[0]->u.number == expected_value);
}

void test_pop_one_integer() {
    int expected_return = 0;
    int expected_top = 0;
    int expected_ltype = NUMBER;
    int expected_value = 42;
    int actual_return;

    Token token = {NUMBER, {42}};
    Token out_token;
    Stack *stack = stack_initialize();

    stack_push(stack, &token);
    actual_return = stack_pop(stack, &out_token);

    assert(actual_return == expected_return);
    assert(stack->top == expected_top);
    assert(out_token.ltype == expected_ltype);
    assert(out_token.u.number == expected_value);
}

void test_all() {
    test_pop_empty_stack();
    test_push_one_integer();
}

int main() {
    test_all();

    return 0;
}