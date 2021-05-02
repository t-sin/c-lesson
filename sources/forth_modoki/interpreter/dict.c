#include <assert.h>
#include <stdio.h>

#include "parser.h"
#include "dict.h"
#include "util.h"

static int dict_pos = 0;

typedef struct DictEntry {
    char *key;
    Token value;
} DictEntry;

#define DICT_SIZE 1024
static DictEntry dict_array[DICT_SIZE];

void dict_reset() {
    dict_pos = 0;
}

void dict_put(char *key, Token *token) {
    int entry_pos = -1;
    for (int pos = 0; pos < dict_pos; pos++) {
        if (streq(dict_array[pos].key, key)) {
            entry_pos = pos;
            break;
        }
    }

    if (entry_pos != -1) {
        dict_array[entry_pos].key = key;
        dict_array[entry_pos].value.ltype = token->ltype;
        dict_array[entry_pos].value.u = token->u;

    } else {
        dict_array[dict_pos].key = key;
        dict_array[dict_pos].value.ltype = token->ltype;
        dict_array[dict_pos].value.u = token->u;
        dict_pos++;
    }
}

int dict_get(char *key, Token *out_token) {
    for (int i = 0; i < DICT_SIZE; i++) {
        if (streq(dict_array[i].key, key)) {
            out_token->ltype = dict_array[i].value.ltype;
            out_token->u = dict_array[i].value.u;
            return DICT_FOUND;
        }
    }

    return DICT_NOT_FOUND;
}

void dict_print_all() {
    printf("[dict] dict_pos = %d\n", dict_pos);
    for (int i = 0; i < dict_pos; i++) {
        printf("[dict] %s: ", dict_array[i].key);
        print_token(&dict_array[i].value);
    }
}

static void test_dict_put_one_integer() {
    Token input = {NUMBER, {42}};
    char *key = "test";
    int expected_dict_pos = 1;

    dict_reset();
    dict_put(key, &input);

    assert(dict_pos == expected_dict_pos);
    assert(dict_array[0].value.ltype == input.ltype);
    assert(dict_array[0].value.u.number == input.u.number);
}

static void test_dict_put_one_literal_name() {
    Token input = {LITERAL_NAME, { .name = "foo"}};
    char *key = "aaa";
    int expected_dict_pos = 1;

    dict_reset();
    dict_put(key, &input);

    assert(dict_pos == expected_dict_pos);
    assert(dict_array[0].value.ltype == input.ltype);
    assert(dict_array[0].value.u.name == input.u.name);
}

static void test_dict_put_two_different_names() {
    Token input1 = {LITERAL_NAME, { .name = "foo"}};
    Token input2 = {EXECUTABLE_NAME, { .name = "bar"}};
    int expected_dict_pos1 = 1;
    int expected_dict_pos2 = 2;

    dict_reset();

    dict_put(input1.u.name, &input1);
    assert(dict_pos == expected_dict_pos1);
    assert(dict_array[expected_dict_pos1 - 1].value.ltype == input1.ltype);
    assert(dict_array[expected_dict_pos1 - 1].value.u.name == input1.u.name);

    dict_put(input2.u.name, &input2);
    assert(dict_pos == expected_dict_pos2);
    assert(dict_array[expected_dict_pos2 - 1].value.ltype == input2.ltype);
    assert(dict_array[expected_dict_pos2 - 1].value.u.name == input2.u.name);
}

static void test_dict_put_two_same_names() {
    Token input1 = {LITERAL_NAME, { .name = "foo"}};
    Token input2 = {EXECUTABLE_NAME, { .name = "foo"}};
    int expected_dict_pos1 = 1;
    int expected_dict_pos2 = 1;

    dict_reset();

    dict_put(input1.u.name, &input1);
    assert(dict_pos == expected_dict_pos1);
    assert(dict_array[expected_dict_pos1 - 1].value.ltype == input1.ltype);
    assert(dict_array[expected_dict_pos1 - 1].value.u.name == input1.u.name);

    dict_put(input2.u.name, &input2);
    assert(dict_pos == expected_dict_pos2);
    assert(dict_array[expected_dict_pos2 - 1].value.ltype == input2.ltype);
    assert(dict_array[expected_dict_pos2 - 1].value.u.name == input2.u.name);
}

static void test_dict_get_one_integer() {
    Token input = {NUMBER, {42}};
    int expected_ret = DICT_FOUND;
    int expected_dict_pos = 1;

    char *key = "test";
    Token output;

    dict_reset();
    dict_put(key, &input);

    int ret = dict_get(key, &output);
    assert(ret == expected_ret);
    assert(dict_pos == expected_dict_pos);
    assert(output.ltype == input.ltype);
    assert(output.u.number == input.u.number);
}

static void test_dict_get_one_literal_name() {
    Token input = {LITERAL_NAME, { .name = "name" }};
    int expected_ret = DICT_FOUND;
    int expected_dict_pos = 1;

    Token output;

    dict_reset();
    dict_put(input.u.name, &input);

    int ret = dict_get(input.u.name, &output);
    assert(ret == expected_ret);
    assert(dict_pos == expected_dict_pos);
    assert(output.ltype == input.ltype);
    assert(streq(output.u.name, input.u.name));
}

#ifdef DICT_TEST
int main() {

    test_dict_put_one_integer();
    test_dict_put_one_literal_name();
    test_dict_put_two_different_names();
    test_dict_put_two_same_names();
    test_dict_get_one_integer();
    test_dict_get_one_literal_name();

    return 0;
}
#endif
