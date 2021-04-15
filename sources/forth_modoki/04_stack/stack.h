#include <stdlib.h>
#include "token.h"

#define STACK_SIZE 1024

typedef struct StackEntry {
    Token *token;
} StackEntry;

typedef struct Stack {
    StackEntry array[STACK_SIZE];
    int top;
} Stack;

#define STACK_FULL -1
#define STACK_EMPTY -1

// スタックを初期化して返す。freeは必要ないけど適宜。
Stack* stack_initialize() {
    // スタックつくる
    // スタック返す
    return NULL;
}

// スタックに要素を追加する。
// 返り値はtopの位置。スタックがいっぱいの状態でプッシュするとSTACK_FULLが返る。
int stack_push(Stack *stack) {
    return 0;
}

// スタックから要素を取り出す。
// 返り値はスタックトップの位置。スタックが空の状態でポップするとSTACK_EMPTYが返る。
int stack_pop(Stack *stack, StackEntry *out_entry) {
    return 0;
}
