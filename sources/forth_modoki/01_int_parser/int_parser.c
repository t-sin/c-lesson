#include <stdio.h>
#include <assert.h>

static const char* const input = "123 456  1203";

int is_number(int ch) {
    return '0' <= ch && ch <= '9';
}

int is_not_number(int ch) {
    return !is_number(ch);
}

int skip(int offset, int (*pred)(int)) {
    int ch;
    int n = offset;
    while (ch = input[n], ch != 0) {
        if (!pred(ch)) {
            return n;
        }
        n++;
    }
    return n;
}

int my_pow(int base, int exp) {
    int num = 1;
    for (int n = 1; n < exp; n++) {
        num *= base;
    }
    return num;
}

int parse_int(int start, int length) {
    int digit;
    int ch;
    int num = 0;

    for (int n = 0; n < length; n++) {
        ch = input[start + n];
        digit = my_pow(10, length - n);
        num += (ch - '0') * digit;
    }

    return num;
}

int main() {
    int answer1 = 0;
    int answer2 = 0;
    int answer3 = 0;

    int *answers[] = {&answer1, &answer2, &answer3};
    int pos = 0;
    int start, end;

    for (int n = 0; n < 3; n++) {
        start = skip(pos, is_not_number);
        end = skip(start, is_number);
        // printf("pos = %d\n", pos);
        // printf("s, e, l = %d, %d, %d\n", start, end, end - start);
        // printf("n = %d\n", parse_int(start, end - start));
        *answers[n] = parse_int(start, end - start);
        pos = end + 1;
    }

    // verity result.
    assert(answer1 == 123);
    assert(answer2 == 456);
    assert(answer3 == 1203);

    return 0;
}