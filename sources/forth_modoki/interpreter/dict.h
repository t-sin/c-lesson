#include "element.h"

typedef struct Dict Dict;

#define DICT_FOUND 1
#define DICT_NOT_FOUND 0

Dict* dict_init();
void dict_put(Dict *dict, char *key, Element *e);
int dict_get(Dict *dict, char *key, Element *out_elem);
void dict_print_all(Dict *dict);
