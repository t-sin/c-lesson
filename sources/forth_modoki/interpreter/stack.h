#include "element.h"

typedef struct Stack Stack;

#define STACK_FULL -1
#define STACK_EMPTY -2
#define STACK_OUT_OF_RANGE -3

// スタックを初期化して返す。
Stack* stack_init();

int stack_length(Stack *stack);

// スタックに要素を追加する。
// 返り値はtopの位置。スタックがいっぱいの状態でプッシュするとSTACK_FULLが返る。
int stack_push(Stack *stack, Element *e);

// スタックから要素を取り出す。
// 返り値はスタックトップの位置。スタックが空の状態でポップするとSTACK_EMPTYが返る。
int stack_pop(Stack *stack, Element *out_elem);

// スタックのトップからn番目 (0オリジン) の要素をスタックはいじらずに取得する。
// nがスタックの深さより大きかったらSTACK_OUT_OF_RANGEを返す。
int stack_peek(Stack *stack, int n, Element *out_elem);

void stack_print_all(Stack *stack);
int stack_is_empty(Stack *stack);