#include "clesson.h"
#include <stdlib.h>
#include <string.h>

static const char* input = "123 456";
static int len = 0;
static int pos = 0;


int cl_getc() {
    if(len == pos)
        return EOF;
    return input[pos++];
}

void cl_getc_set_src(char* str){
    input = str;
    len = strlen(str);
    pos = 0;
}

#define BUFFER_SIZE 1024 * 10

void cl_getc_set_file(FILE *fin){
    char *buf = (char *)malloc(sizeof(char) * BUFFER_SIZE);
    memset(buf, 0, sizeof(char) * BUFFER_SIZE);

    int ch;
    int idx = 0;

    while (ch = fgetc(fin), ch != EOF) {
        buf[idx++] = ch;
    }

    input = buf;
    len = strlen(buf);
    pos = 0;
}