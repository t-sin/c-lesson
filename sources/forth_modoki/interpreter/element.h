#ifndef forth_modoki_element
#define forth_modoki_element

typedef enum ElementType {
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME,
    ELEMENT_C_FUNC,
    ELEMENT_COMPILE_FUNC,
    ELEMENT_EXEC_ARRAY
} ElementType;

typedef struct ElementArray ElementArray;
typedef struct Emitter Emitter;

typedef struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
        void (*compile_func)(Emitter *);
        ElementArray *byte_codes;
    } u;
} Element;

typedef struct ElementArray {
    int len;
    Element elements[0];
} ElementArray;

void copy_element(Element *dest, Element *src);
void print_element(Element *e);
int element_equal(Element *a, Element *b);
int element_is_true(Element *e);
int element_is_false(Element *e);

// element emitter for compiler

typedef struct Emitter {
    Element *elems;
    int pos;
} Emitter;

void emit_number(Emitter *emitter, int n);
void emit_exec_name(Emitter *emitter, char *name);

#endif
