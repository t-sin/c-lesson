#include <assert.h>
#include <stdlib.h>

#include "element.h"
#include "continuation.h"

void copy_continuation(Continuation *dest, Continuation *src) {
    switch (src->ctype) {
    case CONT_CONT:
        dest->u.c.exec_array = src->u.c.exec_array;
        dest->u.c.pc = src->u.c.pc;
        break;

    case CONT_ELEMENT:
        copy_element(&dest->u.e, &src->u.e);
        break;
    }
}

void cont_proceed(Continuation *cont, int n) {
    assert(cont->ctype == CONT_CONT);

    cont->u.c.pc += n;
}

#define CONT_MAX_DEPTH 1024

static Continuation co_stack[CONT_MAX_DEPTH];
static int co_stack_pos = 0;

void co_reset() {
    co_stack_pos = 0;
}

int co_length() {
    return co_stack_pos;
}

int co_push(Continuation *cont) {
    if (co_stack_pos < CONT_MAX_DEPTH) {
        co_stack[co_stack_pos].ctype = cont->ctype;

        copy_continuation(&co_stack[co_stack_pos], cont);
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
    copy_continuation(out_cont, &co_stack[co_stack_pos]);

    return co_stack_pos;
}

int co_peek(int n, Continuation *out_cont) {
    if (co_stack_pos == 0) {
        return CONT_EMPTY;
    }

    int pos = co_stack_pos - 1 - n;
    copy_continuation(out_cont, &co_stack[pos]);

    return n;
}

static void test_pop_from_empty_stack() {
    int expected_ret = CONT_EMPTY;

    Continuation cont;

    co_stack_pos = 0;
    int ret = co_pop(&cont);

    assert(ret == expected_ret);
}

static void test_push_one_exec_array() {
    int expected_ret = 1;
    int expected_pos = 1;
    int expected_pc = 42;

    Continuation cont = {CONT_CONT, {NULL, 42}};

    co_stack_pos = 0;
    int ret = co_push(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
    assert(co_stack[0].u.c.pc == expected_pc);
}

static void test_pop_one_exec_array() {
    Continuation input = {CONT_CONT, {NULL, 42}};
    Continuation expected = {CONT_CONT, {NULL, 42}};
    int expected_ret = 0;
    int expected_pos = 0;

    co_stack_pos = 0;
    co_push(&input);

    Continuation cont;
    int ret = co_pop(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
    assert(cont.u.c.pc == expected.u.c.pc);
}

static void test_peek_one_exec_array() {
    Continuation input_cont = {CONT_CONT, {NULL, 42}};
    int input_n = 0;
    Continuation expected = {CONT_CONT, {NULL, 42}};
    int expected_ret = 0;
    int expected_pos = 1;

    co_stack_pos = 0;
    co_push(&input_cont);

    Continuation cont;
    int ret = co_peek(input_n, &cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
    assert(cont.u.c.pc == expected.u.c.pc);
}

static void test_push_to_full_stack() {
    int expected_ret = CONT_FULL;
    int expected_pos = CONT_MAX_DEPTH;

    Continuation cont = {CONT_CONT, {NULL, 32}};
    int ret;

    co_stack_pos = 0;

    for (int i = 0; i <= CONT_MAX_DEPTH - 1; i++) {
        ret = co_push(&cont);
        assert(ret != CONT_FULL);
    }

    ret = co_push(&cont);

    assert(ret == expected_ret);
    assert(co_stack_pos == expected_pos);
}

static void test_all() {
    test_pop_from_empty_stack();
    test_push_one_exec_array();
    test_pop_one_exec_array();
    test_peek_one_exec_array();
    test_push_to_full_stack();
}

#ifdef CONT_TEST
int main() {
    test_all();
    return 0;
}
#endif
