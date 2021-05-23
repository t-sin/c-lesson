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
    C_FUNC,
    UNKNOWN
} LexicalType;

typedef struct Token {
    enum LexicalType ltype;
    union {
        int number;
        char onechar;
        char *name;
        void (*cfunc)();
    } u;
} Token;

void token_copy(Token *dest, Token *src);
void print_token(Token *token);

#endif
