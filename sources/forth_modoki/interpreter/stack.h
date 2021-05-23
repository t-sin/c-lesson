#include "token.h"
#include "parser.h"

typedef struct Stack Stack;

#define STACK_FULL -1
#define STACK_EMPTY -2

// スタックを初期化して返す。
Stack* stack_init();

// スタックに要素を追加する。
// 返り値はtopの位置。スタックがいっぱいの状態でプッシュするとSTACK_FULLが返る。
int stack_push(Stack *stack, Token *token);

// スタックから要素を取り出す。
// 返り値はスタックトップの位置。スタックが空の状態でポップするとSTACK_EMPTYが返る。
int stack_pop(Stack *stack, Token *out_token);

void stack_print_all(Stack *stack);
