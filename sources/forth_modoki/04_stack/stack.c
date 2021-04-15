#include <assert.h>
#include <stdlib.h>
#include "stack.h"

#define STACK_SIZE 1024

Stack* stack_initialize() {
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    stack->array = (StackEntry *)malloc(sizeof(StackEntry) * STACK_SIZE);
    stack->top = 0;

    return stack;
}

int stack_push(Stack *stack, StackEntry *entry) {
    return 0;
}

int stack_pop(Stack *stack, StackEntry *out_entry) {
    if (stack->top == 0) {
        return STACK_EMPTY;
    }

    return 0;
}

void test_pop_empty_stack() {
    int expected_top = STACK_EMPTY;
    int actual_top;
    StackEntry entry;

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