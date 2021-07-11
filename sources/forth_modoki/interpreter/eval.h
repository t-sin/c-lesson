#ifndef forth_modoki_eval
#define forth_modoki_eval

// evalの前に呼び、スタックや辞書の準備をする
void initialize_eval();

// cl_getc()からPostScript文字列を読んで実行する
// initialize_eval()とcl_getc()の入力が設定されている前提で動く
void eval();

// (たとえばeval()後の) スタック状態を表示する
void print_stack();

#endif
