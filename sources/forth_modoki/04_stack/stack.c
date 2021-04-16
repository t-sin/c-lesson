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


    Stack *stack = stack_initialize();
    actual_top = stack_pop(stack, &entry);

    assert(expected_top == actual_top);
}

void test() {
    test_pop_empty_stack();
}

int main() {
    test();

    return 0;
}