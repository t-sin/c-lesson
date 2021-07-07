#ifndef forth_modoki_element
#define forth_modoki_element

typedef enum ElementType {
    ELEMENT_NUMBER,
    ELEMENT_EXECUTABLE_NAME,
    ELEMENT_LITERAL_NAME,
    ELEMENT_C_FUNC
} ElementType;

typedef struct Element {
    enum ElementType etype;
    union {
        int number;
        char *name;
        void (*cfunc)();
    } u;
} Element;

void copy_element(Element *dest, Element *src);
void print_element(Element *e);

#endif
