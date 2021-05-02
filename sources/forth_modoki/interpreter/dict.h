#include "parser.h"

typedef struct Dict Dict;

#define DICT_FOUND 1
#define DICT_NOT_FOUND 0

void dict_put(char *key, Token *token);
int dict_get(char *key, Token *out_token);
void dict_print_all();
