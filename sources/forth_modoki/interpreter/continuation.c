#include <assert.h>
#include <stdlib.h>

#include "continuation.h"

#define CONT_MAX_DEPTH 1024

static Continuation co_stack[CONT_MAX_DEPTH];
static int co_stack_pos = 0;

int co_push(Continuation *cont) {
    if (co_stack_pos < CONT_MAX_DEPTH) {
        co_stack[co_stack_pos].exec_array = cont->exec_array;
        co_stack[co_stack_pos].pc = cont->pc;
        co_stack_pos++;
        return co_stack_pos;
    }

    return CONT_FULL;
}

int co_pop(Continuation *out_cont) {
    if (co_stack_pos == 0) {
        return CONT_EMPTY;
    }

    co_stack_pos--;
    out_cont->exec_array = co_stack[co_stack_pos].exec_array;
    out_cont->pc = co_stack[co_stack_pos].pc;

    return co_stack_pos;
}

void test_pop_from_empty_stack() {
    int expected_ret = CONT_EMPTY;

    Continuation cont;

    co_stack_pos = 0;
    int ret = co_pop(&cont);

    assert(ret == expected_ret);
}

void test_push_one_exec_array() {
    int expected_ret = 1;
    int expected_pos = 1;
    int expected_pc = 42;

    Continuation cont = {NULL, 42};

    co_stack_pos = 0;
    int ret = co_push(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
    assert(co_stack[0].pc == expected_pc);
}

void test_pop_one_exec_array() {
    Continuation input = {NULL, 42};
    Continuation expected = {NULL, 42};
    int expected_ret = 0;
    int expected_pos = 0;

    co_stack_pos = 0;
    co_push(&input);

    Continuation cont;
    int ret = co_pop(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
    assert(cont.pc == expected.pc);
}

void test_push_to_full_stack() {
    int expected_ret = CONT_FULL;
    int expected_pos = CONT_MAX_DEPTH;

    Continuation cont = {NULL, 32};
    int ret;

    for (int i = 0; i <= CONT_MAX_DEPTH - 1; i++) {
        ret = co_push(&cont);
        assert(ret != CONT_FULL);
    }

    ret = co_push(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
}

void test_all() {
    test_pop_from_empty_stack();
    test_push_one_exec_array();
    test_pop_one_exec_array();
    test_push_to_full_stack();
}

#ifdef CONT_TEST
int main() {
    test_all();
    return 0;
}
#endif
