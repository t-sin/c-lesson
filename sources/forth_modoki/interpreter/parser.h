#ifndef forth_modoki_parser
#define forth_modoki_parser

typedef enum LexicalType {
    NUMBER,
    SPACE,
    EXECUTABLE_NAME,
    LITERAL_NAME,
    OPEN_CURLY,
    CLOSE_CURLY, 
    END_OF_FILE,
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

int parse_one(int prev_ch, Token *out_token);

#endif
