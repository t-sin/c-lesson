#include "token.h"

typedef struct StackEntry {
    Token *token;
} StackEntry;

typedef struct Stack {
    StackEntry *array;
    int top;
} Stack;

#define STACK_FULL -1
#define STACK_EMPTY -2

// スタックを初期化して返す。
Stack* stack_initialize();

// スタックに要素を追加する。
// 返り値はtopの位置。スタックがいっぱいの状態でプッシュするとSTACK_FULLが返る。
int stack_push(Stack *stack, StackEntry *entry);

// スタックから要素を取り出す。
// 返り値はスタックトップの位置。スタックが空の状態でポップするとSTACK_EMPTYが返る。
int stack_pop(Stack *stack, StackEntry *out_entry);
