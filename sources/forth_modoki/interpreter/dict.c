#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token.h"
#include "parser.h"
#include "dict.h"
#include "util.h"

int hash(char *str) {
   unsigned int val = 0;
   while(*str) {
       val += *str++;
   }
   return (int)(val%1024);
}

typedef struct DictEntry {
    char *key;
    Token value;
    struct DictEntry *next;
} DictEntry;

typedef struct Dict {
    DictEntry **array;
    size_t length;
} Dict;

#define DICT_SIZE 1024

Dict* dict_init() {
    Dict *dict = (Dict *)malloc(sizeof(Dict));

    size_t size = sizeof(DictEntry*) * DICT_SIZE;

    dict->array = (DictEntry **)malloc(size);
    memset(dict->array, 0, size);
    dict->length = DICT_SIZE;

    return dict;
}

DictEntry* new_dict_entry(char *key, Token *value) {
    DictEntry *entry = (DictEntry *)malloc(sizeof(DictEntry));
    entry->next = NULL;
    entry->key = (char *)malloc(sizeof(strlen(key) + 1));
    strcpy(entry->key, key);
    token_copy(&entry->value, value);
    return entry;
}

void update_or_insert_list(DictEntry *head, char *key, Token *value) {
    while (1) {
        if (streq(head->key, key)) {
            token_copy(&head->value, value);
            return;
        }
        if (head->next == NULL) {
            head->next = new_dict_entry(key, value);
            return;
        }

        head = head->next;
    }
}

void dict_put(Dict *dict, char *key, Token *token) {
    int idx = hash(key);
    DictEntry *head = dict->array[idx];

    if (head == NULL) {
        head = new_dict_entry(key, token);
        dict->array[idx] = head;
        return;
    }

    update_or_insert_list(head, key, token);
}

int dict_get(Dict *dict, char *key, Token *out_token) {
    int idx = hash(key);
    DictEntry *head = dict->array[idx];

    for (; head != NULL; head = head->next) {
        if (streq(head->key, key)) {
            token_copy(out_token, &head->value);
            return DICT_FOUND;
        }
    }

    return DICT_NOT_FOUND;
}

void dict_print_all(Dict *dict) {
    for (int i = 0; i < dict->length; i++) {
        if (dict->array[i] != NULL) {
            DictEntry *head = dict->array[i];
            for (; head != NULL; head = head->next) {
                printf("[dict] %s: ", head->key);
                print_token(&head->value);
            }
        }
    }
}

static void test_dict_put_one_integer() {
    Token input = {NUMBER, {42}};
    char *key = "a";

    Dict *dict = dict_init();
    dict_put(dict, key, &input);

    int idx = hash(key);
    assert(dict->array[idx] != NULL);
    assert(dict->array[idx]->value.ltype == input.ltype);
    assert(dict->array[idx]->value.u.number == input.u.number);
}

static void test_dict_put_one_literal_name() {
    Token input = {LITERAL_NAME, { .name = "foo"}};
    char *key = "aaa";

    Dict *dict = dict_init();
    dict_put(dict, key, &input);

    int idx = hash(key);
    assert(dict->array[idx] != NULL);
    assert(dict->array[idx]->value.ltype == input.ltype);
    assert(dict->array[idx]->value.u.name == input.u.name);
}

static void test_dict_put_two_different_names_but_same_hash_values() {
    // この名前はハッシュ関数 (dict.c) のアルゴリズムに依存する
    // ここでのハッシュ関数は文字列のアスキーコードを足して余りを取るだけなので、
    // 文字を入れ替えただけだと同じハッシュ値になる。
    Token input1 = {LITERAL_NAME, { .name = "foo"}};
    Token input2 = {EXECUTABLE_NAME, { .name = "oof"}};

    Dict *dict = dict_init();
    int idx;

    dict_put(dict, input1.u.name, &input1);
    idx = hash(input1.u.name);
    assert(dict->array[idx] != NULL);
    assert(dict->array[idx]->value.ltype == input1.ltype);
    assert(dict->array[idx]->value.u.name == input1.u.name);

    dict_put(dict, input2.u.name, &input2);
    idx = hash(input2.u.name);
    DictEntry *next = dict->array[idx]->next;
    assert(next != NULL);
    assert(next->value.ltype == input2.ltype);
    assert(next->value.u.name == input2.u.name);
}

static void test_dict_get_one_integer() {
    Token input = {NUMBER, {42}};
    int expected_ret = DICT_FOUND;

    char *key = "test";
    Token output;

    Dict *dict = dict_init();
    dict_put(dict, key, &input);

    int ret = dict_get(dict, key, &output);
    assert(ret == expected_ret);
    assert(output.ltype == input.ltype);
    assert(output.u.number == input.u.number);
}

static void test_dict_get_one_literal_name() {
    Token input = {LITERAL_NAME, { .name = "name" }};
    int expected_ret = DICT_FOUND;

    Token output;

    Dict *dict = dict_init();
    dict_put(dict, input.u.name, &input);

    int ret = dict_get(dict, input.u.name, &output);
    assert(ret == expected_ret);
    assert(output.ltype == input.ltype);
    assert(streq(output.u.name, input.u.name));
}

static void test_dict_get_one_overwritten_integer() {
    char *key = "key";
    Token input1 = {NUMBER, {42}};
    Token input2 = {NUMBER, {420}};
    Token *expected_token = &input2;
    int expected_ret = DICT_FOUND;

    Token output;

    Dict *dict = dict_init();
    dict_put(dict, key, &input1);
    dict_put(dict, key, &input2);

    int ret = dict_get(dict, key, &output);
    assert(ret == expected_ret);
    assert(output.ltype == expected_token->ltype);
    assert(output.u.number == expected_token->u.number);
}

#ifdef DICT_TEST
int main() {

    test_dict_put_one_integer();
    test_dict_put_one_literal_name();
    test_dict_put_two_different_names_but_same_hash_values();
    test_dict_get_one_integer();
    test_dict_get_one_literal_name();

    test_dict_get_one_overwritten_integer();

    return 0;
}
#endif
