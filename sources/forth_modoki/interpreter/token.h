#ifndef forth_modoki_token
#define forth_modoki_token

typedef enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    OPEN_CURLY,
    CLOSE_CURLY, 
    END_OF_FILE,
    COMMENT,
    UNKNOWN
} LexicalType;

typedef struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
    } u;
} Token;

void copy_token(Token *dest, Token *src);
void print_token(Token *token);
int token_equal(Token *a, Token *b);

#endif
